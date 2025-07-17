#include <ccmd/ccmd.hpp>

#include <algorithm>
#include <charconv>
#include <cstring>
#include <map>

namespace {
using namespace ccmd;

struct Header {
    static constexpr const uint32_t MAGIC = 0x44'4D'43'B1u; // &plusmnCMD
    static constexpr const uint32_t VERSION = 1u;

    uint32_t magic = MAGIC;
    uint32_t version = VERSION;
    uint32_t lookupCount = 0;
    uint32_t opcodeCount = 0;
    uint32_t argumentCount = 0;
    uint32_t heapSize = 0;
};

struct Callback {
    Fn* fn{};
    void* ctx{};
};

struct Lookup {
    uint32_t heapPos{};
    uint32_t length{};
};

struct OpCode {
    uint16_t idx{};
    uint16_t argCount{};
    uint32_t argPos{};
};

struct Arg {
    enum Tag : uint8_t {
        eShortString,
        eString,
        eInt32,
        eUint32,
        eFloat,
    };
    struct Data {
        union {
            uint32_t heap;
            uint32_t u;
            int32_t i;
            float f;
        };
        uint32_t heapSize : 24;
        uint32_t tag : 8;
    };

    union {
        Data data{};
        char shortString[ 7 ];
    };
};
static_assert( sizeof( Arg ) == 8 );

template <typename T>
ErrorCode read( std::span<const std::byte>& o, T& t )
{
    if ( o.size() < sizeof( T ) ) return ErrorCode::eBytecodeNotEnoughData;
    std::memcpy( &t, o.data(), sizeof( T ) );
    o = o.subspan( sizeof( T ) );
    return ErrorCode::eSuccess;
}

template <typename T>
ErrorCode read( std::span<const std::byte>& o, std::span<const T>& t, uint32_t size )
{
    auto bytesNeeded = sizeof( T ) * size;
    if ( o.size() < bytesNeeded ) return ErrorCode::eBytecodeNotEnoughData;
    auto data = reinterpret_cast<const T*>( o.data() );
    t = std::span<const T>{ data, data + size };
    o = o.subspan( bytesNeeded );
    return ErrorCode::eSuccess;
}

ErrorCode cast( std::span<const std::byte> o, std::string_view& t, Lookup strinfo )
{
    if ( o.size() < strinfo.heapPos + strinfo.length ) return ErrorCode::eBytecodeNotEnoughData;
    auto data = reinterpret_cast<const char*>( o.data() + strinfo.heapPos );
    t = std::string_view{ data, data + strinfo.length };
    return ErrorCode::eSuccess;
}

uint32_t push( std::pmr::vector<std::byte>& o, std::string_view t )
{
    auto ret = o.size();
    o.resize( o.size() + t.size() + 1 );
    std::memcpy( o.data() + ret, t.data(), t.size() );
    o.back() = {};
    return static_cast<uint32_t>( ret );
}

template <typename T>
uint32_t push( std::pmr::vector<std::byte>& o, const std::pmr::vector<T>& t )
{
    auto ret = o.size();
    o.resize( o.size() + t.size() * sizeof( T ) );
    std::memcpy( o.data() + ret, t.data(), t.size() * sizeof( T ) );
    return static_cast<uint32_t>( ret );
}

bool readLine( std::string_view& s, std::string_view& ret )
{
    if ( s.empty() ) return false;
    auto pos = s.find( '\n' );
    ret = s.substr( 0, pos );
    pos = ( pos != s.npos ) ? pos + 1 : s.size();
    s.remove_prefix( pos );
    return true;
}


struct WordSplitter {
    std::string_view line;

    bool operator () ( std::string_view& ret )
    {
        auto begin = std::find_if_not( line.begin(), line.end(), &isSeparator );
        if ( begin == line.end() ) return false;

        const bool q = ( *begin == '"' );
        std::advance( begin, q );
        const auto end = std::find_if( begin, line.end(), q ? &isQuote : &isSeparator );
        ret = std::string_view{ begin, end };
        auto distance = std::distance( line.begin(), end );
        distance += ( end != line.end() && isQuote( *end ) );
        line.remove_prefix( static_cast<uint32_t>( distance ) );
        return true;
    }

    static bool isSeparator( char c )
    {
        switch ( c ) {
        // case '\n':
        case '\r':
        case ' ':
        case '\t':
        case '\f':
        case '\v':
            return true;
        default:
            return false;
        }
    }

    static bool isQuote( char c )
    {
        return c == '"';
    }
};

}

namespace ccmd {

struct Vm {
    OpCode currentOp{};
    std::span<const Arg> args{};
    std::span<const std::byte> heap{};
    RunContext* rctx{};
    ErrorCode ec = ErrorCode::eSuccess;
};
}

namespace {

template <typename T>
bool argv( Vm* vm, uint32_t i, T& value )
{
    if ( !vm ) [[unlikely]] return false;
    if ( i >= vm->currentOp.argCount ) [[unlikely]] return false;
    auto pos = i + vm->currentOp.argPos;
    if ( pos >= vm->args.size() ) [[unlikely]] return false;
    auto arg = vm->args[ pos ];
    if constexpr ( std::is_trivially_copyable_v<T> ) {
        switch ( arg.data.tag ) {
        case Arg::eInt32: value = static_cast<T>( arg.data.i ); break;
        case Arg::eUint32: value = static_cast<T>( arg.data.u ); break;
        case Arg::eFloat: value = static_cast<T>( arg.data.f ); break;
        case Arg::eShortString:
        case Arg::eString:
            return false;
        [[unlikely]] default:
            vm->ec = ErrorCode::eBytecodeCorrupted;
            return false;
        }
    }
    else if constexpr ( std::is_same_v<T, std::pmr::string> ) {
        char tmp[ 20 ]{};
        std::to_chars_result res{};
        switch ( arg.data.tag ) {
        case Arg::eString:
            if ( arg.data.heap + arg.data.heapSize > vm->heap.size() ) [[unlikely]] {
                vm->ec = ErrorCode::eBytecodeCorrupted;
                return false;
            }
            value.resize( arg.data.heapSize );
            std::memcpy( value.data(), vm->heap.data() + arg.data.heap, arg.data.heapSize );
            return true;

        case Arg::eShortString:
            value = arg.shortString;
            return true;

        case Arg::eInt32: res = std::to_chars( std::begin( tmp ), std::end( tmp ), arg.data.i ); break;
        case Arg::eUint32: res = std::to_chars( std::begin( tmp ), std::end( tmp ), arg.data.u ); break;
        case Arg::eFloat: res = std::to_chars( std::begin( tmp ), std::end( tmp ), arg.data.f ); break;
        [[unlikely]] default:
            vm->ec = ErrorCode::eBytecodeCorrupted;
            return false;
        }
        value.resize( static_cast<size_t>( res.ptr - tmp ) );
        std::memcpy( value.data(), tmp, value.size() );
    }
    return true;
}
}

namespace ccmd {

std::pmr::vector<std::byte> compile( std::string_view stream, const CompileContext& ctx )
{
    std::pmr::vector<std::string_view> words( ctx.alloc );
    std::pmr::vector<Lookup> lookup( ctx.alloc );
    std::pmr::vector<std::byte> heap( ctx.alloc );
    std::pmr::vector<OpCode> opcodes( ctx.alloc );
    std::pmr::vector<Arg> args( ctx.alloc );
    std::pmr::map<std::string_view, uint32_t> map( ctx.alloc );
    words.reserve( 8 );
    lookup.reserve( 64 );
    opcodes.reserve( 255 );
    args.reserve( 255 );
    heap.reserve( 255 );

    uint32_t cmdIndex = 0;
    for ( std::string_view line; readLine( stream, line ); ) {
        words.clear();
        {
            WordSplitter ws{ line };
            std::string_view word{};
            while ( ws( word ) ) {
                words.emplace_back( word );
            }
            if ( words.empty() ) [[unlikely]] continue;
        }
        auto [ it, inserted ] = map.insert( std::make_pair( words.front(), 0u ) );
        if ( inserted ) {
            it->second = cmdIndex++;
            auto heapPos = push( heap, words.front() );
            lookup.emplace_back( Lookup{ .heapPos = heapPos, .length = static_cast<uint32_t>( words.front().size() ) } );
        }

        auto& op = opcodes.emplace_back();
        op.idx = static_cast<uint16_t>( it->second );
        op.argCount = static_cast<uint16_t>( words.size() - 1 );
        op.argPos = static_cast<uint32_t>( args.size() );
        for ( uint32_t i = 1; i < words.size(); ++i ) {
            auto& arg = args.emplace_back();
            auto& str = words[ i ];
            auto end = str.data() + str.size();
            auto OK = std::from_chars_result{ end, {} };
            if ( std::from_chars( str.data(), end, arg.data.i ) == OK ) { arg.data.tag = arg.eInt32; continue; }
            if ( std::from_chars( str.data(), end, arg.data.u ) == OK ) { arg.data.tag = arg.eUint32; continue; }
            if ( std::from_chars( str.data(), end, arg.data.f ) == OK ) { arg.data.tag = arg.eFloat; continue; }

            if ( str.size() <= std::size( arg.shortString ) ) {
                arg.data.tag = arg.eShortString;
                std::memcpy( arg.shortString, str.data(), str.size() );
                continue;
            }
            arg.data.tag = arg.eString;
            arg.data.heapSize = static_cast<uint32_t>( str.size() ) & 0xFFFFFF;
            auto [ onHeap, onHeapInserted ] = map.insert( std::make_pair( str, 0u ) );
            if ( onHeapInserted ) {
                onHeap->second = push( heap, str );
            }
            arg.data.heap = onHeap->second;
        }
    }
    Header header{
        .lookupCount = static_cast<uint32_t>( lookup.size() ),
        .opcodeCount = static_cast<uint32_t>( opcodes.size() ),
        .argumentCount = static_cast<uint32_t>( args.size() ),
        .heapSize = static_cast<uint32_t>( heap.size() ),
    };
    std::pmr::vector<std::byte> bytecode( ctx.alloc );
    bytecode.reserve( sizeof( Header )
        + lookup.size() * sizeof( Lookup )
        + opcodes.size() * sizeof( OpCode )
        + args.size() * sizeof( Arg )
        + heap.size()
    );
    bytecode.resize( sizeof( header ) );
    std::memcpy( bytecode.data(), &header, sizeof( header ) );
    push( bytecode, lookup );
    push( bytecode, opcodes );
    push( bytecode, args );
    push( bytecode, heap );
    return bytecode;
}

ErrorCode run( std::span<const Command> commands, std::span<const std::byte> bytecode, RunContext* rctx )
{
    auto alloc = ( rctx && rctx->alloc ) ? rctx->alloc : std::pmr::get_default_resource();
    Header header{};
    if ( auto ec = read( bytecode, header ); ec != ErrorCode::eSuccess ) { return ec; }
    if ( header.magic != header.MAGIC ) { return ErrorCode::eBytecodeVersionMismatch; }
    if ( header.version != header.VERSION ) { return ErrorCode::eBytecodeVersionMismatch; }

    Vm vm{
        .rctx = rctx,
    };
    std::span<const Lookup> lookup{};
    std::span<const OpCode> opcodes{};
    if ( auto ec = read( bytecode, lookup, header.lookupCount ); ec != ErrorCode::eSuccess ) { return ec; }
    if ( auto ec = read( bytecode, opcodes, header.opcodeCount ); ec != ErrorCode::eSuccess ) { return ec; }
    if ( auto ec = read( bytecode, vm.args, header.argumentCount ); ec != ErrorCode::eSuccess ) { return ec; }
    if ( auto ec = read( bytecode, vm.heap, header.heapSize ); ec != ErrorCode::eSuccess ) { return ec; }

    std::pmr::vector<Callback> callbacks( header.lookupCount, alloc );
    for ( uint32_t i = 0; i < header.lookupCount; ++i ) {
        std::string_view cmdName{};
        if ( auto ec = cast( vm.heap, cmdName, lookup[ i ] ); ec != ErrorCode::eSuccess ) { return ec; }

        auto it = std::ranges::find_if( commands, [cmdName]( const auto& cmd ) { return cmd.name == cmdName; } );
        if ( it != commands.end() ) {
            callbacks[ i ].fn = it->callback;
            callbacks[ i ].ctx = it->ctx;
            continue;
        }

        it = std::ranges::find_if( commands, []( const auto& cmd ) { return cmd.name.empty(); } );
        if ( it != commands.end() ) {
            callbacks[ i ].fn = it->callback;
            callbacks[ i ].ctx = it->ctx;
            continue;
        }
        if ( rctx ) {
            rctx->commandName = cmdName;
        }
        return ErrorCode::eUnresolvedCommand;
    }

    for ( auto&& it : opcodes ) {
        if ( it.idx >= header.opcodeCount ) [[unlikely]] {
            return ErrorCode::eBytecodeCorrupted;
        }
        vm.currentOp = it;
        auto& cb = callbacks[ it.idx ];
        auto ec = cb.fn( &vm, cb.ctx );
        if ( vm.ec == ErrorCode::eSuccess && ec == 0u ) [[likely]] continue;
        if ( !rctx ) return ErrorCode::eFunctionFail;

        auto cmd = std::ranges::find_if( commands, [fn=cb.fn]( const auto& cmd ) { return cmd.callback == fn; } );
        rctx->commandName = cmd->name;
        rctx->commandExitCode = ec;
        return ErrorCode::eFunctionFail;
    }

    return ErrorCode::eSuccess;
}

void setError( Vm* vm, std::string_view message )
{
    if ( !vm ) return;
    if ( !vm->rctx ) return;
    vm->rctx->commandErrorMessage = message;
}

uint32_t argc( Vm* vm )
{
    return vm ? vm->currentOp.argCount : 0;
}

bool argv( Vm* vm, uint32_t i, bool& value )
{
    return ::argv<bool>( vm, i, value );
}

bool argv( Vm* vm, uint32_t i, int32_t& value )
{
    return ::argv<int32_t>( vm, i, value );
}

bool argv( Vm* vm, uint32_t i, uint32_t& value )
{
    return ::argv<uint32_t>( vm, i, value );
}

bool argv( Vm* vm, uint32_t i, float& value )
{
    return ::argv<float>( vm, i, value );
}

bool argv( Vm* vm, uint32_t i, std::pmr::string& value )
{
    return ::argv<std::pmr::string>( vm, i, value );
}

}
