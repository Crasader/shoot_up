#ifndef Popup_h
#define Popup_h

#include <stdio.h>
#include "cocos2d.h"
#include "ui/CocosGUI.h"

using namespace cocos2d;

namespace UICustom {
    
    
    class PopupDelegates : public cocos2d::LayerRadialGradient
    {
        
    public:
        static PopupDelegates *create();
        
    protected:
        virtual bool        init();
        virtual void        setUpTouches();
        virtual void        show(const bool animated = true);
        virtual void        dismiss(const bool animated = true);
        void                setAnimated(const bool animated){_isAnimated = animated;};
        
        cocos2d::ui::ImageView *_bg = nullptr;
    private:
        bool _isAnimated = false;
    };
    
    
    
    class Popup : public PopupDelegates
    {
        
    public:
        static Popup* create(const std::string &title, const std::string &msg, const std::function<void ()> &YesFunc);
        static void increase_speed(MenuItemFont *, MenuItemFont *, MenuItemFont *);
        static void increase_power(MenuItemFont *, MenuItemFont *, MenuItemFont *);
        
    private:
        void initBg();
    };
    
}

#endif /* Popup_h */

