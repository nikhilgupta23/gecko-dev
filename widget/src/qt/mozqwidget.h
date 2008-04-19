#ifndef MOZQWIDGET_H
#define MOZQWIDGET_H

#include <qwidget.h>
#include "nsIWidget.h"

class QEvent;
class nsWindow;

class MozQWidget : public QWidget
{
    Q_OBJECT
public:
    MozQWidget(nsWindow* receiver, QWidget *parent,
               const char *name, int f);

    /**
     * Mozilla helper.
     */
    void setModal(bool);
    bool SetCursor(nsCursor aCursor);
    void dropReciever() { mReceiver = 0x0; };
    nsWindow* getReciever() { return mReceiver; };

protected:
    virtual bool event(QEvent *ev);
private:
    nsWindow *mReceiver;
};

#endif
