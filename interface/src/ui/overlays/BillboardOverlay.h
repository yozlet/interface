//
//  BillboardOverlay.h
//
//
//  Created by Clement on 7/1/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_BillboardOverlay_h
#define hifi_BillboardOverlay_h

#include <TextureCache.h>

#include "Planar3DOverlay.h"
#include "PanelAttachable.h"

class BillboardOverlay : public Planar3DOverlay, public PanelAttachable {
    Q_OBJECT
public:
    static QString const TYPE;
    virtual QString getType() const { return TYPE; }

    BillboardOverlay();
    BillboardOverlay(const BillboardOverlay* billboardOverlay);

    virtual void render(RenderArgs* args);

    virtual void update(float deltatime);

    // setters
    void setURL(const QString& url);
    void setIsFacingAvatar(bool isFacingAvatar) { _isFacingAvatar = isFacingAvatar; }

    virtual void setProperties(const QScriptValue& properties);
    void setClipFromSource(const QRect& bounds) { _fromImage = bounds; }
    virtual QScriptValue getProperty(const QString& property);

    virtual bool findRayIntersection(const glm::vec3& origin, const glm::vec3& direction, float& distance, BoxFace& face);
    
    virtual BillboardOverlay* createClone() const;

protected:
    void setTransforms(Transform* transform);

private:
    void setBillboardURL(const QString& url);

    QString _url;
    NetworkTexturePointer _texture;
    
    QRect _fromImage; // where from in the image to sample

    bool _isFacingAvatar = true;
};

#endif // hifi_BillboardOverlay_h
