#ifndef KORG_PICTUREDECORATION_H
#define KORG_PICTUREDECORATION_H
// $Id$

namespace KOrg {

class PictureDecoration {
  public:
    PictureDecoration() {};
    virtual ~PictureDecoration() {};
    
    virtual QPixmap day(QDate &date) = 0;
};

}
