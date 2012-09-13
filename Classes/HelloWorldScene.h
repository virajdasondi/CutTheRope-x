#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "Box2D/Box2D.h"

USING_NS_CC;

class VRope;

class HelloWorld : public cocos2d::CCLayer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // there's no 'id' in cpp, so we recommand to return the exactly class pointer
    static cocos2d::CCScene* scene();
    
    // a selector callback
    virtual void menuCloseCallback(CCObject* pSender);

    // implement the "static node()" method manually
    LAYER_NODE_FUNC(HelloWorld);

    void initPhysics();

    b2Body* createCandyAt(cocos2d::CCPoint pt,bool userData);

    void createRopeD(b2Body* bodyA,b2Vec2 anchorA,b2Body* bodyB,b2Vec2 anchorB,float32 sag);
    void initLevel();

    void update(ccTime dt);

    //Check intersection..
	bool checkLineIntersection(cocos2d::CCPoint p1,cocos2d::CCPoint p2,cocos2d::CCPoint p3,cocos2d::CCPoint p4);

	//Touches..
	void ccTouchesMoved(CCSet* touches, CCEvent* event);

	//Create tip body..
	b2Body* createRopeTipBody();

	//Croc attitude..
//	void changeCrocAttitude();

private:
    b2World* world;
    cocos2d::CCSprite *croc_;

    std::vector<VRope *> *ropes;
    std::vector<b2Body *> *candies;

    b2Body* groundBody;    // weak ref
	cocos2d::CCSpriteBatchNode *ropeSpriteSheet; // weak ref

	b2Body *crocMouth_;          // weak ref
	b2Fixture *crocMouthBottom_;    // weak ref

	bool crocMouthOpened;
	cocos2d::CCTimer *crocAttitudeTimer;

};

#endif // __HELLOWORLD_SCENE_H__
