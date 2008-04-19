/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:expandtab:shiftwidth=4:tabstop=4:
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Christopher Blizzard
 * <blizzard@mozilla.org>.  Portions created by the Initial Developer
 * are Copyright (C) 2001 the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Masayuki Nakano <masayuki@d-toybox.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __nsWindow_h__
#define __nsWindow_h__

#include "nsAutoPtr.h"

#include "nsCommonWidget.h"

#include "nsWeakReference.h"

#include "nsIDragService.h"
#include "nsITimer.h"
#include "nsWidgetAtoms.h"


#ifdef Q_WS_X11
#include <QX11Info>
#endif

class QEvent;

class nsWindow : public nsCommonWidget, public nsSupportsWeakReference
{
public:
    nsWindow();
    virtual ~nsWindow();

    static void ReleaseGlobals();

    NS_DECL_ISUPPORTS_INHERITED

    // nsIWidget
    NS_IMETHOD         Create(nsIWidget        *aParent,
                              const nsRect     &aRect,
                              EVENT_CALLBACK   aHandleEventFunction,
                              nsIDeviceContext *aContext,
                              nsIAppShell      *aAppShell,
                              nsIToolkit       *aToolkit,
                              nsWidgetInitData *aInitData);
    NS_IMETHOD         Create(nsNativeWidget aParent,
                              const nsRect     &aRect,
                              EVENT_CALLBACK   aHandleEventFunction,
                              nsIDeviceContext *aContext,
                              nsIAppShell      *aAppShell,
                              nsIToolkit       *aToolkit,
                              nsWidgetInitData *aInitData);
    NS_IMETHOD         Destroy(void);
    NS_IMETHOD         SetParent(nsIWidget* aNewParent);
    NS_IMETHOD         SetModal(PRBool aModal);
    NS_IMETHOD         IsVisible(PRBool & aState);
    NS_IMETHOD         ConstrainPosition(PRBool aAllowSlop,
                                         PRInt32 *aX,
                                         PRInt32 *aY);
    NS_IMETHOD         Move(PRInt32 aX,
                            PRInt32 aY);
    NS_IMETHOD         PlaceBehind(nsTopLevelWidgetZPlacement  aPlacement,
                                   nsIWidget                  *aWidget,
                                   PRBool                      aActivate);
    NS_IMETHOD         SetZIndex(PRInt32 aZIndex);
    NS_IMETHOD         SetSizeMode(PRInt32 aMode);
    NS_IMETHOD         Enable(PRBool aState);
    NS_IMETHOD         SetFocus(PRBool aRaise = PR_FALSE);
    NS_IMETHOD         GetScreenBounds(nsRect &aRect);
    NS_IMETHOD         SetForegroundColor(const nscolor &aColor);
    NS_IMETHOD         SetBackgroundColor(const nscolor &aColor);
    NS_IMETHOD         SetCursor(nsCursor aCursor);
    NS_IMETHOD         SetCursor(imgIContainer* aCursor,
                                 PRUint32 aHotspotX, PRUint32 aHotspotY);
    NS_IMETHOD         Validate();
    NS_IMETHOD         Invalidate(PRBool aIsSynchronous);
    NS_IMETHOD         Invalidate(const nsRect &aRect,
                                  PRBool        aIsSynchronous);
    NS_IMETHOD         InvalidateRegion(const nsIRegion *aRegion,
                                        PRBool           aIsSynchronous);
    NS_IMETHOD         Update();
    NS_IMETHOD         SetColorMap(nsColorMap *aColorMap);
    NS_IMETHOD         Scroll(PRInt32  aDx,
                              PRInt32  aDy,
                              nsRect  *aClipRect);
    NS_IMETHOD         ScrollWidgets(PRInt32 aDx,
                                     PRInt32 aDy);
    NS_IMETHOD         ScrollRect(nsRect  &aSrcRect,
                                  PRInt32  aDx,
                                  PRInt32  aDy);
    virtual void*      GetNativeData(PRUint32 aDataType);
    NS_IMETHOD         SetBorderStyle(nsBorderStyle aBorderStyle);
    NS_IMETHOD         SetTitle(const nsAString& aTitle);
    NS_IMETHOD         SetIcon(const nsAString& aIconSpec);
    NS_IMETHOD         SetWindowClass(const nsAString& xulWinType);
    NS_IMETHOD         SetMenuBar(nsIMenuBar * aMenuBar);
    NS_IMETHOD         ShowMenuBar(PRBool aShow);
    NS_IMETHOD         WidgetToScreen(const nsRect& aOldRect,
                                      nsRect& aNewRect);
    NS_IMETHOD         ScreenToWidget(const nsRect& aOldRect,
                                      nsRect& aNewRect);
    NS_IMETHOD         BeginResizingChildren(void);
    NS_IMETHOD         EndResizingChildren(void);
    NS_IMETHOD         EnableDragDrop(PRBool aEnable);
    virtual void       ConvertToDeviceCoordinates(nscoord &aX,
                                                  nscoord &aY);
    NS_IMETHOD         PreCreateWidget(nsWidgetInitData *aWidgetInitData);
    NS_IMETHOD         CaptureMouse(PRBool aCapture);
    NS_IMETHOD         CaptureRollupEvents(nsIRollupListener *aListener,
                                           PRBool aDoCapture,
                                           PRBool aConsumeRollupEvent);
    NS_IMETHOD         GetAttention(PRInt32 aCycleCount);
    NS_IMETHOD         MakeFullScreen(PRBool aFullScreen);
    NS_IMETHOD         HideWindowChrome(PRBool aShouldHide);

    // utility methods
    void               LoseFocus();
    qint32             ConvertBorderStyles(nsBorderStyle aStyle);

protected:
    /**
     * Event handlers (proxied from the actual qwidget).
     * They follow normal Qt widget semantics.
     */
    void Initialize(QWidget *widget);
    friend class nsQtEventDispatcher;
    friend class InterceptContainer;
    friend class MozQWidget;

    virtual bool OnExposeEvent(QPaintEvent *);
    virtual bool OnConfigureEvent(QMoveEvent *);
    virtual bool OnSizeAllocate(QResizeEvent *);
    virtual bool OnDeleteEvent(QCloseEvent *);
    virtual bool OnEnterNotifyEvent(QEvent *);
    virtual bool OnLeaveNotifyEvent(QEvent *);
    virtual bool OnMotionNotifyEvent(QMouseEvent *);
    virtual bool OnButtonPressEvent(QMouseEvent *);
    virtual bool OnButtonReleaseEvent(QMouseEvent *);
    virtual bool mouseDoubleClickEvent(QMouseEvent *);
    virtual bool OnContainerFocusInEvent(QFocusEvent *);
    virtual bool OnContainerFocusOutEvent(QFocusEvent *);
    virtual bool OnKeyPressEvent(QKeyEvent *);
    virtual bool OnKeyReleaseEvent(QKeyEvent *);
    virtual bool OnScrollEvent(QWheelEvent *);

    virtual bool contextMenuEvent(QContextMenuEvent *);
    virtual bool imStartEvent(QEvent *);
    virtual bool imComposeEvent(QEvent *);
    virtual bool imEndEvent(QEvent *);
    virtual bool OnDragEnter (QDragEnterEvent *);
    virtual bool OnDragMotionEvent(QDragMoveEvent *);
    virtual bool OnDragLeaveEvent(QDragLeaveEvent *);
    virtual bool OnDragDropEvent (QDropEvent *);
    virtual bool showEvent(QShowEvent *);
    virtual bool hideEvent(QHideEvent *);

    bool               OnWindowStateEvent(QEvent *aEvent);

    nsresult           NativeCreate(nsIWidget        *aParent,
                                    nsNativeWidget    aNativeParent,
                                    const nsRect     &aRect,
                                    EVENT_CALLBACK    aHandleEventFunction,
                                    nsIDeviceContext *aContext,
                                    nsIAppShell      *aAppShell,
                                    nsIToolkit       *aToolkit,
                                    nsWidgetInitData *aInitData);

    void               NativeResize(PRInt32 aWidth,
                                    PRInt32 aHeight,
                                    PRBool  aRepaint);

    void               NativeResize(PRInt32 aX,
                                    PRInt32 aY,
                                    PRInt32 aWidth,
                                    PRInt32 aHeight,
                                    PRBool  aRepaint);

    void               NativeShow  (PRBool  aAction);

    void               EnsureGrabs  (void);
    void               GrabKeyboard (void);
    void               ReleaseGrabs (void);
    void               GrabPointer(void);

    enum PluginType {
        PluginType_NONE = 0,   /* do not have any plugin */
        PluginType_XEMBED,     /* the plugin support xembed */
        PluginType_NONXEMBED   /* the plugin does not support xembed */
    };

    void               SetPluginType(PluginType aPluginType);
    void               SetNonXEmbedPluginFocus(void);
    void               LoseNonXEmbedPluginFocus(void);

    void               ThemeChanged(void);

    NS_IMETHOD         BeginResizeDrag   (nsGUIEvent* aEvent, PRInt32 aHorizontal, PRInt32 aVertical);

   void                ResizeTransparencyBitmap(PRInt32 aNewWidth, PRInt32 aNewHeight);
   void                ApplyTransparencyBitmap();
   NS_IMETHOD          SetHasTransparentBackground(PRBool aTransparent);
   NS_IMETHOD          GetHasTransparentBackground(PRBool& aTransparent);
   nsresult            UpdateTranslucentWindowAlphaInternal(const nsRect& aRect,
                                                            PRUint8* aAlphas, PRInt32 aStride);

   gfxASurface        *GetThebesSurface();

private:
    void               GetToplevelWidget(QWidget **aWidget);
    void               SetUrgencyHint(QWidget *top_window, PRBool state);
    void              *SetupPluginPort(void);
    nsresult           SetWindowIconList(const nsCStringArray &aIconList);
    void               SetDefaultIcon(void);
    void               InitButtonEvent(nsMouseEvent &aEvent, QMouseEvent *aEvent, int aClickCount = 1);
    PRBool             DispatchCommandEvent(nsIAtom* aCommand);
    QWidget           *createQWidget(QWidget *parent, nsWidgetInitData *aInitData);

    QWidget            *mDrawingarea;

    PRUint32            mIsVisible : 1,
                        mRetryPointerGrab : 1,
                        mActivatePending : 1,
                        mRetryKeyboardGrab : 1;
    PRInt32             mSizeState;
    PluginType          mPluginType;

    PRInt32             mTransparencyBitmapWidth;
    PRInt32             mTransparencyBitmapHeight;

    nsRefPtr<gfxASurface> mThebesSurface;

    // Transparency
    PRBool       mIsTransparent;
    // This bitmap tracks which pixels are transparent. We don't support
    // full translucency at this time; each pixel is either fully opaque
    // or fully transparent.
    char*       mTransparencyBitmap;
 
    // all of our DND stuff
    // this is the last window that had a drag event happen on it.
    void   InitDragEvent         (nsMouseEvent &aEvent);

    // this is everything we need to be able to fire motion events
    // repeatedly
    PRUint32 mKeyDownFlags[8];

    /* Helper methods for DOM Key Down event suppression. */
    PRUint32* GetFlagWord32(PRUint32 aKeyCode, PRUint32* aMask) {
        /* Mozilla DOM Virtual Key Code is from 0 to 224. */
        NS_ASSERTION((aKeyCode <= 0xFF), "Invalid DOM Key Code");
        aKeyCode &= 0xFF;

        /* 32 = 2^5 = 0x20 */
        *aMask = PRUint32(1) << (aKeyCode & 0x1F);
        return &mKeyDownFlags[(aKeyCode >> 5)];
    }

    PRBool IsKeyDown(PRUint32 aKeyCode) {
        PRUint32 mask;
        PRUint32* flag = GetFlagWord32(aKeyCode, &mask);
        return ((*flag) & mask) != 0;
    }

    void SetKeyDownFlag(PRUint32 aKeyCode) {
        PRUint32 mask;
        PRUint32* flag = GetFlagWord32(aKeyCode, &mask);
        *flag |= mask;
    }

    void ClearKeyDownFlag(PRUint32 aKeyCode) {
        PRUint32 mask;
        PRUint32* flag = GetFlagWord32(aKeyCode, &mask);
        *flag &= ~mask;
    }
    PRInt32 mQCursor;

};

class nsChildWindow : public nsWindow
{
public:
    nsChildWindow();
    ~nsChildWindow();

  PRInt32 mChildID;
};

class nsPopupWindow : public nsWindow
{
public:
  nsPopupWindow ();
  ~nsPopupWindow ();

  PRInt32 mChildID;
};
#endif /* __nsWindow_h__ */

