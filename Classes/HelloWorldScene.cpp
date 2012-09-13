#include "HelloWorldScene.h"
#include "vrope-x/VRope.h"

USING_NS_CC;


CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::node();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::node();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }

    setIsTouchEnabled(true);

    ropes = new vector<VRope*>;
	candies = new vector<b2Body*>;

	ropeSpriteSheet = CCSpriteBatchNode::batchNodeWithFile("Images/rope_texture.png");
	this->addChild(ropeSpriteSheet);

    // Load the sprite sheet into the sprite cache
	cocos2d::CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("Images/CutTheVerlet.plist");

	// Add the background
	cocos2d::CCSprite *background = cocos2d::CCSprite::spriteWithSpriteFrameName("bg.png");
	background->setAnchorPoint(cocos2d::CCPointZero);
	this->addChild(background,-1);

	// Add the croc
	croc_ = cocos2d::CCSprite::spriteWithSpriteFrameName("croc_front_mouthclosed.png");
	croc_->setAnchorPoint(CCPointMake(1.0, 0.0));

	croc_->setPosition(CCPointMake(320.0, 30.0));
	this->addChild(croc_,1);

	this->initPhysics();
	this->initLevel();

	scheduleUpdate();	//For simulation..
    
    return true;
}

void HelloWorld::initPhysics()
{
    CCSize s = CCDirector::sharedDirector()->getWinSize();

    b2Vec2 gravity;
    gravity.Set(0.0f, -10.0f);
    world = new b2World(gravity);

    // Do we want to let bodies sleep?
    world->SetAllowSleeping(true);

    world->SetContinuousPhysics(true);

//     m_debugDraw = new GLESDebugDraw( PTM_RATIO );
//     world->SetDebugDraw(m_debugDraw);

    uint32 flags = 0;
    flags += b2Draw::e_shapeBit;
    //        flags += b2Draw::e_jointBit;
    //        flags += b2Draw::e_aabbBit;
    //        flags += b2Draw::e_pairBit;
    //        flags += b2Draw::e_centerOfMassBit;
    //m_debugDraw->SetFlags(flags);


    // Define the ground body.
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0, 0); // bottom-left corner

    // Call the body factory which allocates memory for the ground body
    // from a pool and creates the ground box shape (also from a pool).
    // The body is also added to the world.
    groundBody = world->CreateBody(&groundBodyDef);

    // Define the ground box shape.
    b2EdgeShape groundBox;

    // bottom
    groundBox.Set(b2Vec2(0,0), b2Vec2(s.width/PTM_RATIO,0));
    groundBody->CreateFixture(&groundBox,0);

    // top
    groundBox.Set(b2Vec2(0,s.height/PTM_RATIO), b2Vec2(s.width/PTM_RATIO,s.height/PTM_RATIO));
    groundBody->CreateFixture(&groundBox,0);

    // left
    groundBox.Set(b2Vec2(0,s.height/PTM_RATIO), b2Vec2(0,0));
    groundBody->CreateFixture(&groundBox,0);

    // right
    groundBox.Set(b2Vec2(s.width/PTM_RATIO,s.height/PTM_RATIO), b2Vec2(s.width/PTM_RATIO,0));
    groundBody->CreateFixture(&groundBox,0);

    // Define the croc's "mouth".
	b2BodyDef crocBodyDef;
	crocBodyDef.position.Set((s.width - croc_->getTextureRect().size.width)/PTM_RATIO, (croc_->getPosition().y)/PTM_RATIO);

	crocMouth_ = world->CreateBody(&crocBodyDef);

	// Define the croc's box shape.
	b2EdgeShape crocBox;

	// bottom
	crocBox.Set(b2Vec2(5.0/PTM_RATIO,15.0/PTM_RATIO), b2Vec2(45.0/PTM_RATIO,15.0/PTM_RATIO));
	crocMouthBottom_ = crocMouth_->CreateFixture(&crocBox,0);

	crocMouth_->SetActive(false);
}

b2Body* HelloWorld::createCandyAt(CCPoint pt,bool userData)
{
	// Get the sprite from the sprite sheet
	cocos2d::CCSprite* sprite = cocos2d::CCSprite::spriteWithSpriteFrameName("pineapple.png");
	this->addChild(sprite);

	// Defines the body of your candy
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = b2Vec2(pt.x/PTM_RATIO,pt.y/PTM_RATIO);
	bodyDef.userData = sprite;
	bodyDef.linearDamping = 0.3f;
	b2Body* body = world->CreateBody(&bodyDef);

	// Define the fixture as a polygon
	b2FixtureDef fixtureDef;
	b2PolygonShape spriteShape;

	b2Vec2 verts[] = {
	        b2Vec2(-7.6f / PTM_RATIO, -34.4f / PTM_RATIO),
	        b2Vec2(8.3f / PTM_RATIO, -34.4f / PTM_RATIO),
	        b2Vec2(15.55f / PTM_RATIO, -27.15f / PTM_RATIO),
	        b2Vec2(13.8f / PTM_RATIO, 23.05f / PTM_RATIO),
	        b2Vec2(-3.35f / PTM_RATIO, 35.25f / PTM_RATIO),
	        b2Vec2(-16.25f / PTM_RATIO, 25.55f / PTM_RATIO),
	        b2Vec2(-15.55f / PTM_RATIO, -23.95f / PTM_RATIO)
	    };

	spriteShape.Set(verts,7);

	/*if(userData == true)
		fixtureDef.userData = (void*)1;*/

	fixtureDef.shape = &spriteShape;
	fixtureDef.density = 30.0f;
	fixtureDef.filter.categoryBits = 0x01;
	fixtureDef.filter.maskBits = 0x01;

	body->CreateFixture(&fixtureDef);

	candies->push_back(body);
	/*if(userData == true)
		enemies->push_back(body);*/

	return body;
}

//Creating ropes..
//Creating smooth rope..
void HelloWorld::createRopeD(b2Body* bodyA,b2Vec2 anchorA,b2Body* bodyB,b2Vec2 anchorB,float32 sag)
{
	b2RopeJointDef jd;
	jd.bodyA = bodyA;
	jd.bodyB = bodyB;
	jd.localAnchorA = anchorA;
	jd.localAnchorB = anchorB;

	// Max length of joint = current distance between bodies * sag
	float32 ropeLength = (bodyA->GetWorldPoint(anchorA) - bodyB->GetWorldPoint(anchorB)).Length()*sag;
	jd.maxLength = ropeLength;

	//Create joints..
	b2RopeJoint* ropeJoint = (b2RopeJoint*)world->CreateJoint(&jd);

	VRope* newRope = new VRope(ropeJoint,ropeSpriteSheet);

	ropes->push_back(newRope);

}

//Initializing level..
//Init Level..
void HelloWorld::initLevel()
{
	CCSize s = CCDirector::sharedDirector()->getWinSize();

	// Add the candy
	b2Body* body1 = this->createCandyAt(ccp(s.width*0.5f,s.height*0.7f),false);

	// Add a bunch of ropes
	this->createRopeD(groundBody,b2Vec2((s.width*0.15f)/PTM_RATIO, (s.height*0.8f)/PTM_RATIO),body1,body1->GetLocalCenter(),1.1f);
	this->createRopeD(groundBody,b2Vec2((s.width*0.85f)/PTM_RATIO, (s.height*0.8f)/PTM_RATIO),body1,body1->GetLocalCenter(),1.1f);
	this->createRopeD(groundBody,b2Vec2((s.width*0.83f)/PTM_RATIO, (s.height*0.6f)/PTM_RATIO),body1,body1->GetLocalCenter(),1.1f);


	// Advance the world by a few seconds to stabilize everything.
    int n = 10 * 60;
	int32 velocityIterations = 8;
	int32 positionIterations = 1;
	float32 dt = 1.0 / 60.0;
	while (n--)
	{
		// Instruct the world to perform a single step of simulation.
		world->Step(dt, velocityIterations, positionIterations);
		std::vector<VRope *>::iterator rope;
		for(rope = ropes->begin();rope != ropes->end();++rope)
		{
			(*rope)->update(dt);
		}
	}

	// This last update takes care of the texture repositioning.
	this->update(dt);

	/*crocMouthOpened = true;
	this->changeCrocAttitude();*/
}

//Scheduling...
void HelloWorld::update(ccTime dt)
{
    // TODO : FIXED TIME STEP IMPLEMENTATION
    //It is recommended that a fixed time step is used with Box2D for stability
    //of the simulation, however, we are using a variable time step here.
    //You need to make an informed choice, the following URL is useful
    //http://gafferongames.com/game-physics/fix-your-timestep/

    int velocityIterations = 8;
    int positionIterations = 1;

    // Instruct the world to perform a single step of simulation. It is
    // generally best to keep the time step and iterations fixed.
    world->Step(dt, velocityIterations, positionIterations);

    //Iterate over the bodies in the physics world
    for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
    {
        if (b->GetUserData() != NULL) {
            //Synchronize the AtlasSprites position and rotation with the corresponding body
            CCSprite* myActor = (CCSprite*)b->GetUserData();
            myActor->setPosition( CCPointMake( b->GetPosition().x * PTM_RATIO, b->GetPosition().y * PTM_RATIO) );
            myActor->setRotation( -1 * CC_RADIANS_TO_DEGREES(b->GetAngle()) );
        }
    }

    //For ropes simulation..
	std::vector<VRope *>::iterator rope;
	for(rope = ropes->begin();rope != ropes->end();++rope)
	{

		(*rope)->update(dt);
		(*rope)->updateSprites();
	}

}

//Check intersection..
bool HelloWorld::checkLineIntersection(cocos2d::CCPoint p1,cocos2d::CCPoint p2,cocos2d::CCPoint p3,cocos2d::CCPoint p4)	//Watch Out!!..Class::
{
	// http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
	    CCFloat denominator = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);

	    // In this case the lines are parallel so you assume they don't intersect
	    if (denominator == 0.0f)
	        return false;
	    CCFloat ua = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x)) / denominator;
	    CCFloat ub = ((p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x)) / denominator;

	    if (ua >= 0.0 && ua <= 1.0 && ub >= 0.0 && ub <= 1.0)
	    {
	        return true;
	    }

	    return false;
}

//Touches...
void HelloWorld::ccTouchesMoved(CCSet* touches, CCEvent* event)
{
	CCSize s = CCDirector::sharedDirector()->getWinSize();

	CCTouch *myTouch = (CCTouch *)touches->anyObject();
	CCPoint pt0 = myTouch->previousLocationInView();
	CCPoint pt1 = myTouch->locationInView();					//Watch Ot!!..locationInView() has no argument..

	// Correct Y axis coordinates to cocos2d coordinates
	pt0.y = s.height - pt0.y;
	pt1.y = s.height - pt1.y;
	std::vector<VRope *>::iterator rope;
	for(rope = ropes->begin();rope != ropes->end();++rope)
	    {
			std::vector<VStick *>::iterator stick;
	        for (stick = (*rope)->getSticks().begin();stick != (*rope)->getSticks().end();++stick)
	        {
	        	CCPoint pa = (*stick)->getPointA()->point();
	        	CCPoint pb = (*stick)->getPointB()->point();
	            if (this->checkLineIntersection(pt0,pt1,pa,pb))
	            {
	                // Cut the rope here
	            	b2Body *newBodyA = createRopeTipBody();
					b2Body *newBodyB = createRopeTipBody();

					VRope *newRope = (*rope)->cutRopeInStick((*stick),newBodyA,newBodyB);
					ropes->push_back(newRope);
	                return;
	            }
	        }
	    }
}

//Creating Rope Tip Body..
b2Body* HelloWorld::createRopeTipBody()
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.linearDamping = 0.5f;
	b2Body *body = world->CreateBody(&bodyDef);

	b2FixtureDef circleDef;
	b2CircleShape circle;
	circle.m_radius = 1.0/PTM_RATIO;
	circleDef.shape = &circle;
	circleDef.density = 10.0f;

	// Since these tips don't have to collide with anything
	// set the mask bits to zero
	circleDef.filter.maskBits = 0;
	body->CreateFixture(&circleDef);

	return body;
}

//Croc attitude..
/*void HelloWorld::changeCrocAttitude()
{
    crocMouthOpened = !crocMouthOpened;
    const char* spriteName = crocMouthOpened ?"croc_front_mouthopen.png":"croc_front_mouthclosed.png";
//    [croc_ setDisplayFrame:[[CCSpriteFrameCache sharedSpriteFrameCache] spriteFrameByName:spriteName]];
    croc_->setDisplayFrame(cocos2d::CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(spriteName));
//    [croc_ setZOrder:crocMouthOpened ? 1 : -1];
//    croc_->m_nZOrder = crocMouthOpened ? 1 : -1;

    if(crocMouthOpened)
    	croc_->getParent()->reorderChild(croc_,1);
    else
    	croc_->getParent()->reorderChild(croc_,-1);

    crocMouth_->SetActive(crocMouthOpened);

//    [crocAttitudeTimer invalidate];
//    crocAttitudeTimer->update()
//    [crocAttitudeTimer release];
    crocAttitudeTimer->initWithTarget(this, callfunc_selector(HelloWorld::changeCrocAttitude),2,1,3.0 + 2.0 * CCRANDOM_0_1());
    crocAttitudeTimer = [[NSTimer scheduledTimerWithTimeInterval:3.0 + 2.0 * CCRANDOM_0_1()
                                                          target:self
                                                        selector:@selector(changeCrocAttitude)
                                                        userInfo:nil
                                                         repeats:NO] retain];
}*/

void HelloWorld::menuCloseCallback(CCObject* pSender)
{
    CCDirector::sharedDirector()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
