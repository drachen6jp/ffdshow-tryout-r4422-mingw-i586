#ifndef _CRECT_H_
#define _CRECT_H_

template <class number_t, class POINT>
struct CPointT :POINT {
    CPointT() {
        this->x=0;
        this->y=0;
    }
    CPointT(number_t Ix,number_t Iy) {
        this->x=Ix;
        this->y=Iy;
    }
    inline void operator-=(const CPointT &pt) {
        this->x-=pt.x;
        this->y-=pt.y;
    }

    inline void scale(number_t xnum,number_t xden,number_t ynum,number_t yden) {
        if (xden==0 || yden==0) {
            return;
        }
        this->x=xnum*this->x/xden;
        this->y=ynum*this->y/yden;
    }

    bool operator != (const CPointT &rt) const {
        return !(this->x == rt.x && this->y == rt.y);
    }

    bool operator == (const CPointT &rt) const {
        return (this->x == rt.x && this->y == rt.y);
    }

    bool operator < (const CPointT &rt) const {
        if (this->y < rt.y) {
            return true;
        }
        if (this->y > rt.y) {
            return false;
        }
        if (this->x < rt.x) {
            return true;
        }
        return false;
    }

    bool operator > (const CPointT &rt) const {
        if (this->y > rt.y) {
            return true;
        }
        if (this->y < rt.y) {
            return false;
        }
        if (this->x > rt.x) {
            return true;
        }
        return false;
    }

    void operator = (const CPointT &rt) {
        this->x = rt.x;
        this->y = rt.y;
    }

    void operator = (const POINT &rt) {
        this->x = rt.x;
        this->y = rt.y;
    }
};

template <class number_t, class CSize>
struct CSizeT :CSize {
    CSizeT() {}
    CSizeT(number_t Icx,number_t Icy) {
        this->cx=Icx;
        this->cy=Icy;
    }
    // Operations
    BOOL operator==(CSize size) {
        return (this->cx==size.cx && this->cy==size.cy);
    }
    BOOL operator!=(CSize size) {
        return (this->cx!=size.cx || this->cy!=size.cy);
    }
};

template <class number_t, class RECT, class CPoint, class CSize>
struct CRectT :RECT {
    CRectT() {
        this->top=0;
        this->left=0;
        this->right=0;
        this->bottom=0;
    }
    CRectT(const RECT &r) {
        this->top=r.top;
        this->left=r.left;
        this->bottom=r.bottom;
        this->right=r.right;
    }
    CRectT(number_t Ileft,number_t Itop,number_t Iright,number_t Ibottom) {
        this->left=Ileft;
        this->right=Iright;
        this->top=Itop;
        this->bottom=Ibottom;
    }
    CRectT(const CPoint &topleft,const CPoint &rightbottom) {
        this->left=topleft.x;
        this->top=topleft.y;
        this->right=rightbottom.x;
        this->bottom=rightbottom.y;
    }
    CRectT(const CPoint &topleft,const CSize &size) {
        this->left=topleft.x;
        this->top=topleft.y;
        this->right=this->left+size.cx;
        this->bottom=this->top+size.cy;
    }
    inline void operator&=(const CRectT &rect2) {
        RECT rect1=*this;
        IntersectRect(this,&rect1,&rect2);
    }

    inline void operator += (const CPoint &pt) {
        this->left+=pt.x;
        this->top+=pt.y;
        this->right+=pt.x;
        this->bottom+=pt.y;
    }

    inline void operator -= (const CPoint &pt) {
        this->left-=pt.x;
        this->top-=pt.y;
        this->right-=pt.x;
        this->bottom-=pt.y;
    }

    inline CRectT operator + (const CRectT &rt) {
        return CRectT (this->left + rt.left, this->top + rt.top, this->right + rt.right, this->bottom + rt.bottom);
    }

    inline CRectT operator - (const CRectT &rt) {
        return CRectT (this->left - rt.left, this->top - rt.top, this->right - rt.right, this->bottom - rt.bottom);
    }
    inline bool operator < (const CRectT &rt) const {
        if (this->top < rt.top) {
            return true;
        }
        if (this->top > rt.top) {
            return false;
        }
        if (this->left < rt.left) {
            return true;
        }
        if (this->left > rt.left) {
            return false;
        }
        if (this->bottom < rt.bottom) {
            return true;
        }
        if (this->bottom > rt.bottom) {
            return false;
        }
        if (this->right < rt.right) {
            return true;
        }
        if (this->right > rt.right) {
            return false;
        }
        return false;
    }
    inline bool operator > (const CRectT &rt) const {
        if (this->top < rt.top) {
            return false;
        }
        if (this->top > rt.top) {
            return true;
        }
        if (this->left < rt.left) {
            return false;
        }
        if (this->left > rt.left) {
            return true;
        }
        if (this->bottom < rt.bottom) {
            return false;
        }
        if (this->bottom > rt.bottom) {
            return true;
        }
        if (this->right < rt.right) {
            return false;
        }
        if (this->right > rt.right) {
            return true;
        }
        return false;
    }
    inline bool operator == (const CRectT &rt) const {
        return
            this->top == rt.top
            &&   this->left == rt.left
            && this->bottom == rt.bottom
            &&  this->right == rt.right;
    }
    inline bool operator != (const CRectT &rt) const {
        return !(*this == rt);
    }
    inline number_t Width(void) const {
        return this->right-this->left;
    }
    inline number_t Height(void) const {
        return this->bottom-this->top;
    }
    inline CPoint TopLeft(void) const {
        return CPoint(this->left,this->top);
    }
    inline CPoint BottomRight(void) const {
        return CPoint(this->right,this->bottom);
    }
    inline CSize Size(void) const {
        return CSize(Width(),Height());
    }
    inline void scale(number_t xnum,number_t xden,number_t ynum,number_t yden) {
        if (xden==0 || yden==0) {
            return;
        }
        this->left=xnum*this->left/xden;
        this->top=ynum*this->top/yden;
        this->right=xnum*this->right/xden;
        this->bottom=ynum*this->bottom/yden;
    }
    bool checkOverlap(CRectT &rt) const {
        if (this->top > rt.bottom) {
            return false;
        }
        if (this->bottom < rt.top) {
            return false;
        }
        if (this->left > rt.right) {
            return false;
        }
        if (this->right < rt.left) {
            return false;
        }
        return true;
    }
};

struct POINT_DOUBLE {
    double x,y;
};

struct SIZE_DOUBLE {
    double cx,cy;
};

struct RECT_DOUBLE {
    double left, top, right, bottom;
};

typedef CPointT<int, POINT> CPoint;
typedef CPointT<double, POINT_DOUBLE> CPointDouble;
typedef CSizeT<int, SIZE> CSize;
typedef CSizeT<double, SIZE_DOUBLE> CSizeDouble;
typedef CRectT<int, RECT, CPoint, CSize> CRect;
typedef CRectT<double, RECT_DOUBLE, CPointDouble, CSizeDouble> CRectDouble;

#endif
