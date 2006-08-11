#include "kreversiview.h"

#include <QGraphicsScene>

#include <kdebug.h>

const int BOARD_SIZE = 8;

KReversiView::KReversiView( QGraphicsScene* scene, QWidget *parent )
    : QGraphicsView(scene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setFixedSize( sceneRect().toRect().size() );
}

/* 
QSize KReversiView::sizeHint() const
{
    return QSize( scene()->width()+8, scene()->height()+8 );
}
 */
