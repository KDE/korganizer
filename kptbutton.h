#ifndef KPTBUTTON_H
#define KPTBUTTON_H

#include <qpixmap.h>
#include <qbutton.h>

/**
 * Provides active-raise/lower buttons. w/optional text label beside
 */
class KPTButton : public QButton
{
    Q_OBJECT
public:
    KPTButton( QWidget *_parent = 0L, const char *name = 0L );
    ~KPTButton();
    void setText(const QString &text);
    QString text() const;
    QSize sizeHint() const;
protected:
    virtual void leaveEvent( QEvent *_ev );
    virtual void enterEvent( QEvent *_ev );
        
    virtual void drawButton( QPainter *_painter );
    virtual void drawButtonLabel( QPainter *_painter );

    void paint( QPainter *_painter );

 private:    
    bool raised;    
    QString textLabel;
};

#endif
