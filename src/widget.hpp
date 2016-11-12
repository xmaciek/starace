#pragma once

struct Alignment {
    enum Horizontal { Left, Center, Right };
    enum Vertical { Top, Middle, Bottom };
};

class Widget {
protected:
    double m_width, m_height;
    double m_x, m_y;
    double m_xOffset, m_yOffset;
    bool m_isVisible;
    Alignment::Horizontal m_alignmentHorizontal;
    Alignment::Vertical m_alignmentVertical;

    void moveToPosition() const;
    virtual void recalcAlignmentOffset();

public:
    Widget();
    virtual ~Widget();
    virtual double width() const;
    virtual double height() const;
    virtual double x() const;
    virtual double y() const;

    virtual void setVisible( bool b );
    virtual bool isVisible() const;

    virtual void draw() const = 0;

    virtual void move( double x, double y );
    virtual void resize( double w, double h );

    virtual void setAlignment( Alignment::Horizontal horizontal, Alignment::Vertical vertical = Alignment::Bottom );

};
