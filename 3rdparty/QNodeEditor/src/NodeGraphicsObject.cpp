#include "NodeGraphicsObject.hpp"

#include "ConnectionGraphicsObject.hpp"
#include "ConnectionState.hpp"
#include "FlowScene.hpp"
#include "Node.hpp"
#include "NodeConnectionInteraction.hpp"
#include "NodeDataModel.hpp"
#include "NodePainter.hpp"
#include "StyleCollection.hpp"

#include <QtWidgets/QGraphicsEffect>
#include <QtWidgets/QtWidgets>
#include <cstdlib>
#include <iostream>
using QtNodes::FlowScene;
using QtNodes::Node;
using QtNodes::NodeGraphicsObject;
NodeGraphicsObject::NodeGraphicsObject(FlowScene &scene, Node &node) : _scene(scene), _node(node), _locked(false), _proxyWidget(nullptr)
{
    _scene.addItem(this);
    setFlag(QGraphicsItem::ItemDoesntPropagateOpacityToChildren, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    auto const &nodeStyle = node.nodeDataModel()->nodeStyle();
    {
        auto effect = new QGraphicsDropShadowEffect;
        effect->setOffset(4, 4);
        effect->setBlurRadius(20);
        effect->setColor(nodeStyle.ShadowColor);
        setGraphicsEffect(effect);
    }
    setOpacity(nodeStyle.Opacity);
    setAcceptHoverEvents(true);
    setZValue(0);
    embedQWidget(true);
    // connect to the move signals to emit the move signals in FlowScene
    auto onMoveSlot = [this] {
        _scene.nodeMoved(_node, pos());
    };
    connect(this, &QGraphicsObject::xChanged, this, onMoveSlot);
    connect(this, &QGraphicsObject::yChanged, this, onMoveSlot);
}
NodeGraphicsObject::~NodeGraphicsObject()
{
    _scene.removeItem(this);
}
Node &NodeGraphicsObject::node()
{
    return _node;
}
Node const &NodeGraphicsObject::node() const
{
    return _node;
}
void NodeGraphicsObject::embedQWidget(bool embed)
{
    NodeGeometry &geom = _node.nodeGeometry();
    _node.nodeDataModel()->setWembed(embed);
    if (auto w = _node.nodeDataModel()->embeddedWidget())
    {
        if (embed)
        {
            if (nullptr == _proxyWidget)
            {
                _proxyWidget = new QGraphicsProxyWidget(this);
                w->setParent(nullptr);
                _proxyWidget->setWidget(w);
                _proxyWidget->setPreferredWidth(5);
                geom.recalculateSize();
                if (w->sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag)
                {
                    // If the widget wants to use as much vertical space as possible, set
                    // it to have the geom's equivalentWidgetHeight.
                    _proxyWidget->setMinimumHeight(geom.equivalentWidgetHeight());
                }
                _proxyWidget->setPos(geom.widgetPosition());
                update();
                _proxyWidget->setOpacity(1.0);
                _proxyWidget->setFlag(QGraphicsItem::ItemIgnoresParentOpacity);
            }
        }
        else
        {
            if (nullptr != _proxyWidget)
            {
                _proxyWidget->setWidget(nullptr);
                QPoint pos = QCursor::pos();
                _proxyWidget->deleteLater();
                _proxyWidget = nullptr;
                connect(this, SIGNAL(destroyed()), w, SLOT(deleteLater()));
                geom.recalculateSize();
                update();
                w->setWindowTitle(_node.nodeDataModel()->caption());
                w->setWindowFlags(Qt::Widget);
                w->move(pos.x(), pos.y());
                w->show();
                w->raise();
            }
        }
    }
    moveConnections();
    scene()->update();
}
QRectF NodeGraphicsObject::boundingRect() const
{
    return _node.nodeGeometry().boundingRect();
}
void NodeGraphicsObject::setGeometryChanged()
{
    prepareGeometryChange();
}
void NodeGraphicsObject::moveConnections() const
{
    NodeState const &nodeState = _node.nodeState();
    for (PortType portType : { PortType::In, PortType::Out })
    {
        auto const &connectionEntries = nodeState.getEntries(portType);
        for (auto const &connections : connectionEntries)
        {
            for (auto &con : connections) con.second->getConnectionGraphicsObject().move();
        }
    }
}
void NodeGraphicsObject::lock(bool locked)
{
    _locked = locked;
    setFlag(QGraphicsItem::ItemIsMovable, !locked);
    setFlag(QGraphicsItem::ItemIsFocusable, !locked);
    setFlag(QGraphicsItem::ItemIsSelectable, !locked);
}
void NodeGraphicsObject::paint(QPainter *painter, QStyleOptionGraphicsItem const *option, QWidget *)
{
    painter->setClipRect(option->exposedRect);
    NodePainter::paint(painter, _node, _scene);
}
QVariant NodeGraphicsObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene())
    {
        moveConnections();
    }
    return QGraphicsItem::itemChange(change, value);
}
void NodeGraphicsObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (_locked)
        return;
    // deselect all other items after this one is selected
    if (!isSelected() && !(event->modifiers() & Qt::ControlModifier))
    {
        _scene.clearSelection();
    }
    for (PortType portToCheck : { PortType::In, PortType::Out })
    {
        NodeGeometry const &nodeGeometry = _node.nodeGeometry();
        // TODO do not pass sceneTransform
        int const portIndex = nodeGeometry.checkHitScenePoint(portToCheck, event->scenePos(), sceneTransform());
        if (portIndex != INVALID)
        {
            NodeState const &nodeState = _node.nodeState();
            std::unordered_map<QUuid, Connection *> connections = nodeState.connections(portToCheck, portIndex);
            // start dragging existing connection
            if (!connections.empty() && portToCheck == PortType::In)
            {
                auto con = connections.begin()->second;
                NodeConnectionInteraction interaction(_node, *con, _scene);
                interaction.disconnect(portToCheck);
            }
            else // initialize new Connection
            {
                if (portToCheck == PortType::Out)
                {
                    auto const outPolicy = _node.nodeDataModel()->portOutConnectionPolicy(portIndex);
                    if (!connections.empty() && outPolicy == NodeDataModel::ConnectionPolicy::One)
                    {
                        _scene.deleteConnection(*connections.begin()->second);
                    }
                }
                // todo add to FlowScene
                auto connection = _scene.createConnection(portToCheck, _node, portIndex);
                _node.nodeState().setConnection(portToCheck, portIndex, *connection);
                connection->getConnectionGraphicsObject().grabMouse();
            }
        }
    }
    auto pos = event->pos();
    auto &geom = _node.nodeGeometry();
    auto &state = _node.nodeState();
    if (_node.nodeDataModel()->resizable() && geom.resizeRect().contains(QPoint(pos.x(), pos.y())))
    {
        state.setResizing(true);
    }
}
void NodeGraphicsObject::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    auto &geom = _node.nodeGeometry();
    auto &state = _node.nodeState();
    if (state.resizing())
    {
        auto diff = event->pos() - event->lastPos();
        if (qAbs(diff.x()) < 1 && qAbs(diff.y()) < 1)
        {
            event->ignore();
            return;
        }
        if (auto w = _node.nodeDataModel()->embeddedWidget())
        {
            prepareGeometryChange();
            const QSize size = w->size();
            QSize newSize = size + QSize(diff.x(), diff.y());
            const QSize minSize = geom.minimumEmbeddedSize();
            const QSize maxSize = geom.maximumEmbeddedSize();
            if ((newSize.width() < minSize.width() && newSize.height() < minSize.height()) ||
                (newSize.width() > maxSize.width() && newSize.height() > maxSize.height()))
            {
                event->ignore();
                return;
            }
            newSize = newSize.expandedTo(minSize);
            newSize = newSize.boundedTo(maxSize);
            w->resize(newSize);
            geom.recalculateSize();
            _proxyWidget->setMinimumSize(newSize);
            _proxyWidget->setMaximumSize(newSize);
            _proxyWidget->setPos(geom.widgetPosition());
            update();
            moveConnections();
            event->accept();
        }
    }
    else
    {
        QGraphicsObject::mouseMoveEvent(event);
        if (event->lastPos() != event->pos())
            moveConnections();
        event->ignore();
    }
    QRectF r = scene()->sceneRect();
    r = r.united(mapToScene(boundingRect()).boundingRect());
    scene()->setSceneRect(r);
}
void NodeGraphicsObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    auto &state = _node.nodeState();
    state.setResizing(false);
    QGraphicsObject::mouseReleaseEvent(event);
    // position connections precisely after fast node move
    moveConnections();
    _scene.nodeClicked(node());
}
void NodeGraphicsObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    // bring all the colliding nodes to background
    QList<QGraphicsItem *> overlapItems = collidingItems();
    for (QGraphicsItem *item : overlapItems)
    {
        if (item->zValue() > 0.0)
        {
            item->setZValue(0.0);
        }
    }
    // bring this node forward
    setZValue(1.0);
    if (auto w = _node.nodeDataModel()->embeddedWidget())
    {
        w->raise();
    }
    _node.nodeGeometry().setHovered(true);
    _node.nodeDataModel()->onNodeHoverEnter();
    update();
    _scene.nodeHovered(node(), event->screenPos());
    event->accept();
}
void NodeGraphicsObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    _node.nodeGeometry().setHovered(false);
    _node.nodeDataModel()->onNodeHoverLeave();
    update();
    _scene.nodeHoverLeft(node());
    event->accept();
}
void NodeGraphicsObject::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    auto pos = event->pos();
    auto &geom = _node.nodeGeometry();
    if (_node.nodeDataModel()->resizable() && geom.resizeRect().contains(QPoint(pos.x(), pos.y())))
    {
        setCursor(QCursor(Qt::SizeFDiagCursor));
    }
    else
    {
        setCursor(QCursor());
    }
    event->accept();
}
void NodeGraphicsObject::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    _scene.nodeDoubleClicked(node());
}
void NodeGraphicsObject::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    _scene.nodeContextMenu(node(), mapToScene(event->pos()));
}