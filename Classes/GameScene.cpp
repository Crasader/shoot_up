#include    <math.h>
#include    <vector>
#include    <iomanip>
#include    <audio/android/jni/cddandroidAndroidJavaEngine.h>
#include    <SimpleAudioEngine.h>
#include    "PluginAdMob/PluginAdMob.h"
#include    "cocos2d.h"
#include    "GameScene.h"
#include    "Square.h"
#include    "Line.h"
#include    "Utils.h"
#include    "ShopScene.h"
#include    "Popup.h"
#include    "GridView.h"
#include    "CustomTableViewCell.h"
#include    "UserLocalStore.h"
#include    "ShootUp.h"
#include    "Circle.h"

using namespace std;
using namespace CocosDenshion;

namespace POPUP_MENU_PATH {
    const char *POWER = "pop_up/power.png";
    const char *POWER_SELECTED = "pop_up/power_selected.png";
    const char *SPEED = "pop_up/speed.png";
    const char *SPEED_SELECTED = "pop_up/speed_selected.png";
}

USING_NS_CC;

class IMListener : public sdkbox::AdMobListener {
public:
    GameScene *gs;
    virtual void adViewDidReceiveAd(const std::string &name) {
    }

    virtual void
    adViewDidFailToReceiveAdWithError(const std::string &name, const std::string &msg) {
    }

    virtual void adViewWillPresentScreen(const std::string &name) {
    }

    virtual void adViewDidDismissScreen(const std::string &name) {
        if (!sdkbox::PluginAdMob::isAvailable(name) && name == CONTINUE_REWARD_AD_TEST)
            gs->resume_game();
    }

    virtual void adViewWillDismissScreen(const std::string &name) {
    }

    virtual void adViewWillLeaveApplication(const std::string &name) {
    }

    virtual void reward(const std::string &name, const std::string &currency, double amount) {
    }

};

Scene *GameScene::createScene() {
    auto scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setGravity(Vec2(0, 0));
    scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_NONE);
    auto layer = GameScene::create();
    scene->addChild(layer);
    return scene;
}

bool GameScene::init() {
    if (!Scene::init())
        return false;
    init_main_variable();
    return true;
}

void GameScene::init_main_variable() {
    auto winSize = Director::getInstance()->getVisibleSize();
    check_first_launch();
    game_state = MENU;
    x_screen = winSize.width;
    y_screen = winSize.height;
    if (UserLocalStore::get_achievement_variable(SOUND) == SOUND_OFF)
        sound_activated = false;
    check_first_open();
    hit_played = 0;
    launch_played = 0;
    options_state = OPTIONS_HIDE;
    bullet_state = 0;
    init_ui_components();
    init_listeners();
    init_pool_objects();
    game_menu = get_main_menu();
    addChild(game_menu);
    if (UserLocalStore::get_achievement_variable(FROM_SHOP) == NOT_FROM_SHOP)
        main_menu_coming_animation();
    else
        UserLocalStore::store_achievement_variable(FROM_SHOP, NOT_FROM_SHOP);
    init_options_menu();
    init_bonus_components();
    if (!game_audio)
        game_audio = SimpleAudioEngine::getInstance();
}

void GameScene::check_first_open() {
    if (UserLocalStore::get_achievement_variable(APP_FIRST_OPEN) == ALREADY_OPEN)
        return;
    Sprite *splash_background = Sprite::create(SPLASH_BACKGROUND_TEXTURE);
    Label *logo = Label::createWithTTF("AAA", LOGO_FONT, 100);
    Label *logo_studio = Label::createWithTTF("studio", LOGO_FONT, 30);
    logo->setPosition(Vec2(x_screen / 2, y_screen / 2 + logo->getContentSize().height));
    logo_studio->setPosition(Vec2(x_screen / 2, static_cast<float>(logo->getPositionY() -
                                                                   (logo->getContentSize().height /
                                                                    2 +
                                                                    1.1 *
                                                                    logo_studio->getContentSize().height /
                                                                    2))));
    splash_background->setContentSize(Size(x_screen, y_screen));
    splash_background->setPosition(Vec2(x_screen / 2, y_screen / 2));
    addChild(splash_background, 200);
    addChild(logo, 201);
    addChild(logo_studio, 202);
    auto delay = DelayTime::create(1.5f);
    auto remove = RemoveSelf::create();
    auto sequence = Sequence::create(delay, remove, nullptr);
    splash_background->runAction(sequence);
    logo->runAction(sequence->clone());
    logo_studio->runAction(sequence->clone());
    UserLocalStore::store_achievement_variable(APP_FIRST_OPEN, ALREADY_OPEN);
}

void GameScene::init_ui_components() {
    player = Utils::get_player();
    this->addChild(player);
    score = Label::createWithTTF("test", FIRE_UP_FONT_NUMBERS, 90);
    score->setPosition(Vec2(x_screen / 2, static_cast<float>(y_screen - 0.12 * y_screen)));
    score->setVisible(false);
    addChild(score, 10);
    best_img = Sprite::create(BEST_SCORE_IMG);
    best_img->setPosition(Vec2(x_screen + best_img->getContentSize().height, score->getPositionY() -
                                                                             (score->getContentSize().height /
                                                                              2 +
                                                                              best_img->getContentSize().height /
                                                                              3)));
    best_img->setVisible(false);
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile(BULLETS_PLIST);
    bullet_batch_node = SpriteBatchNode::create(DEFAULT_BULLET_TEXTURE_PLIST);
    bullet_batch_node->setTag(BULLET_BATCH_TAG);
    addChild(bullet_batch_node, 10);
    addChild(best_img, 20);
}

void GameScene::init_listeners() {
    IMListener *imListener = new IMListener;
    imListener->gs = this;
    sdkbox::PluginAdMob::setListener(imListener);
    EventListenerTouchOneByOne *listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(GameScene::onTouchMoved, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, player);
}

void GameScene::init_pool_objects() {
    int index_struct = COMPLEX_STRUCT_ELMTS;
    pool_container = new Line *[27];
    pool_circle = new Circle *[15];
    bullet_container = new Bullet *[101];
    active_lines = new int[5];

    pool_container[0] = Line::create(STARTUP_LINE_2);
    pool_container[1] = Line::create(STARTUP_LINE_3);
    pool_container[2] = Line::create(STARTUP_LINE_4);
    pool_container[3] = Line::create(STARTUP_LINE_5);

    for (int i = 0; i < 15; i++) {
        pool_circle[i] = Circle::create();
        addChild(pool_circle[i]);
    }
    pool_circle[15] = NULL;
    for (int i = 0; i < 4; i++)
        active_lines[i] = EMPTY_VALUE;
    active_lines[4] = '\0';
    for (int j = 0; j < 100; j++) {
        bullet_container[j] = Bullet::create();
        bullet_batch_node->addChild(bullet_container[j]);
    }
    bullet_container[100] = NULL;
    for (int i = 4; i < 7; i++)
        pool_container[i] = Line::create(SIMPLE_LINE_4);
    for (int i = 7; i < 10; i++)
        pool_container[i] = Line::create(SIMPLE_LINE_5);
    for (int i = 10; i < 26; i += 2, index_struct++) {
        pool_container[i] = Line::create(index_struct);
        pool_container[i + 1] = Line::create(index_struct);
    }
    pool_container[26] = NULL;
    for (int i = 0; pool_container[i] != NULL; i++)
        addChild(pool_container[i]);
}

void GameScene::init_bonus_components() {
    bonus_container = new Sprite *[3];
    rect_container = new Sprite *[3];
    shield_rect = Sprite::create(SHIELD_RECT_TEXTURE);
    shield_rect->setScale(player->getContentSize().height /
                          shield_rect->getContentSize().height);
    bonus_container[BONUS_BULLET] = Sprite::createWithSpriteFrameName(DEFAULT_BULLET_TEXTURE);
    bonus_container[BONUS_POWER] = Sprite::createWithSpriteFrameName(DEFAULT_POWER_TEXTURE);
    bonus_container[BONUS_SPEED] = Sprite::createWithSpriteFrameName(DEFAULT_SPEED_TEXTURE);
    rect_container[RECT_BULLET] = Sprite::create(BULLET_RECT);
    rect_container[RECT_POWER] = Sprite::create(POWER_RECT);
    rect_container[RECT_SPEED] = Sprite::create(SPEED_RECT);
    addChild(shield_rect);
    shield_rect->setVisible(false);
    for (int i = 0; i < 3; i++) {
        bonus_container[i]->setVisible(false);
        rect_container[i]->setVisible(false);
        rect_container[i]->setScale(1.1);
        addChild(bonus_container[i]);
        addChild(rect_container[i], 15);
    }
}

void GameScene::init_options_menu() {
    if (UserLocalStore::get_achievement_variable(SOUND) == SOUND_OFF)
        sound = Sprite::create(OPTIONS_SOUND_ON);
    else
        sound = Sprite::create(OPTIONS_SOUND_OFF);
    tuto = Sprite::create(OPTIONS_TUTO);
    sound->setScale(0.9);
    tuto->setScale(0.9);
    sound->setPosition(
            Vec2(options_btn->getPosition().x +
                 options_btn->getContentSize().height,
                 options_btn->getPosition().y));
    tuto->setPosition(Vec2(sound->getPosition().x + sound->getContentSize().height,
                           options_btn->getPosition().y));
    sound->setVisible(false);
    tuto->setVisible(false);
    this->addChild(tuto);
    this->addChild(sound);
}

void GameScene::value_to_update() {
    if (game_score > UserLocalStore::get_achievement_variable(SCORE))
        UserLocalStore::store_achievement_variable(SCORE, game_score);
    UserLocalStore::store_achievement_variable(POINT,
                                               UserLocalStore::get_achievement_variable(POINT) +
                                               game_score);
}

void GameScene::end_of_game() {
    if (game_state != GAME_RUNNING)
        return;
    if (game_shooter_type == SHIELD_TANK && !shield_live_used) {
        shield_live_used = true;
        shield_rect->runAction(Utils::get_shield_blink_animation());
        destroy_complete_line(last_line_hited_id,
                              pool_container[last_line_hited_id]->getPositionY());
        return;
    }
    if (game_shooter_type == SHIELD_TANK) {
        shield_live_used = false;
        shield_rect->setVisible(false);
    }
    show_particle_explode(Vec2(player->getPositionX(), player->getPositionY()), RED, MAX_PARTICLE);
    hit_played = 0;
    launch_played = 0;
    player->setScale(1);
    remove_bonus();
    value_to_update();
    game_state = GAME_END;
    score->setVisible(false);
    player->setVisible(false);
    if (best_img->isVisible()) {
        best_img->setVisible(false);
        best_img->setPositionX(x_screen + best_img->getContentSize().height);
    }
    stop_bullet_shoot();
    if (!game_already_resumed) {
        display_end_menu();
    } else {
        end_menu = get_end_game_menu();
        addChild(end_menu);
    }
}

void GameScene::display_end_menu() {
    game_state = RESUME;
    continue_button = Sprite::createWithSpriteFrameName(DEFAULT_CONTINUE_TEXTURE);
    continue_button->setScale(1.2);
    next_button = Sprite::create(NEXT_BUTTON_TEXTURE);
    next_button->setPosition(
            Vec2(x_screen / 2, static_cast<float>(y_screen / 2 - (0.4 * (y_screen / 2)))));
    continue_button->setPosition(Vec2(x_screen / 2, next_button->getPositionY() +
                                                    continue_button->getContentSize().height));
    addChild(continue_button);
    auto delay = DelayTime::create(6.0f);
    auto callback = CallFuncN::create(
            [&](Node *sender) {
                continue_button->stopAllActions();
                removeChild(continue_button);
                removeChild(next_button);
                game_state = GAME_END;
                end_menu = get_end_game_menu();
                addChild(end_menu);
            });
    wait_sequence = Sequence::create(delay, callback, nullptr);
    runAction(wait_sequence);
    auto scale_up = ScaleTo::create(0.4, static_cast<float>(continue_button->getScale() + 0.1));
    auto scale_down = ScaleTo::create(0.4, static_cast<float>(continue_button->getScale() - 0.1));
    auto blink_seq = Sequence::create(scale_up, scale_down, nullptr);
    continue_button->runAction(Utils::get_continue_anim());
    continue_button->runAction(RepeatForever::create(blink_seq));
    addChild(next_button);
}

void GameScene::reset_arrays() {
    for (int j = 0; active_lines[j] != '\0'; j++)
        active_lines[j] = EMPTY_VALUE;
    for (int i = 0; bullet_container[i] != NULL; i++)
        if (bullet_container[i]->bullet_active)
            bullet_container[i]->reset();
    for (int i = 0; pool_container[i] != NULL; i++) {
        if (pool_container[i]->line_active) {
            pool_container[i]->reset();
        }
    }
}

void GameScene::start_bullet_shoot() {
    float interval = get_shoot_interval();
    if (game_shooter_type == SPEED_TANK) {
        interval -= (interval / 1.5);
    }
    if (bonus_active == BONUS_SPEED)
        interval /= 2;
    this->schedule(schedule_selector(GameScene::launch_bullet), interval);
}

void GameScene::stop_bullet_shoot() {
    unschedule(schedule_selector(GameScene::launch_bullet));
}

void GameScene::surclassement(cocos2d::Ref *pSender) {
    if (game_state != MENU)
        return;
    UICustom::Popup *popup = UICustom::Popup::create("Test 3", "", [=]() {});
    popup->setScale(0);
    popup->setOnExitCallback([&]() {
        if (UserLocalStore::get_achievement_variable(NEW_SHOP_ELEMENT) == 0)
            return;
        auto image_unselected = Sprite::create(SHOP_UNSELECTED_NEW);
        auto image_selected = Sprite::create(SHOP_SELECTED_NEW);
        menu_shop_img->setNormalImage(image_unselected);
        menu_shop_img->setSelectedImage(image_selected);
    });
    auto scaleTo = ScaleTo::create(0.1f, 1.0f);
    addChild(popup);
    popup->runAction(scaleTo);
}

void GameScene::options(cocos2d::Ref *pSender) {
    if (options_state == OPTIONS_HIDE) {
        DelayTime *delayTime = DelayTime::create(0.08);
        tuto->setOpacity(0);
        sound->setOpacity(0);
        auto actionMove = MoveTo::create(0.1,
                                         Vec2(tuto->getPosition().x +
                                              (tuto->getContentSize().width / 6),
                                              tuto->getPosition().y));
        auto actionMove2 = MoveTo::create(0.1, Vec2(sound->getPosition().x +
                                                    (sound->getContentSize().width / 6),
                                                    sound->getPosition().y));
        auto actionRotate = RotateTo::create(0.1, 90);
        options_btn->runAction(Sequence::create(actionRotate, nullptr));
        options_state = OPTIONS_DISPLAYED;
        tuto->setVisible(true);
        sound->setVisible(true);
        auto action = FadeIn::create(0.1);
        auto action2 = FadeIn::create(0.1);
        auto sequence_action = Sequence::create(delayTime, action, NULL);
        tuto->runAction(sequence_action);
        sound->runAction(action2);
        tuto->runAction(actionMove);
        sound->runAction(actionMove2);
    } else {
        DelayTime *delayTime = DelayTime::create(0.1);
        auto actionMove = MoveTo::create(0.1,
                                         Vec2(tuto->getPosition().x -
                                              (tuto->getContentSize().width / 6),
                                              tuto->getPosition().y));
        auto actionMove2 = MoveTo::create(0.1, Vec2(sound->getPosition().x -
                                                    (sound->getContentSize().width / 6),
                                                    sound->getPosition().y));
        auto action = FadeOut::create(0.1);
        auto action2 = FadeOut::create(0.1);
        auto sequence_action = Sequence::create(delayTime, action, NULL);
        tuto->runAction(action2);
        sound->runAction(sequence_action);
        tuto->runAction(actionMove);
        sound->runAction(actionMove2);
        auto actionRotate = RotateTo::create(0.1, 0);
        options_btn->runAction(Sequence::create(actionRotate, nullptr));
        options_state = OPTIONS_HIDE;
    }
}

void GameScene::run_start_animation() {
    auto callback = CallFuncN::create(
            [&](Node *sender) {
                game_menu->cleanup();
                this->removeChild(game_menu);
            });
    auto fadeout = FadeTo::create(0.5, 0);
    auto actionMove1 = MoveBy::create(0.2, Vec2(static_cast<float>(-x_screen * 0.03), 0));
    auto actionMove2 = MoveBy::create(0.2, Vec2(static_cast<float>(x_screen * 0.5), 0));
    auto seq = Sequence::create(actionMove1, actionMove2, nullptr);
    auto actionMove5 = MoveBy::create(0.2, Vec2(static_cast<float>(x_screen * 0.03), 0));
    auto actionMove6 = MoveBy::create(0.2, Vec2(static_cast<float>(-x_screen * 0.5), 0));
    auto actionMove7 = MoveBy::create(0.2, Vec2(0, static_cast<float>(-y_screen * 0.05)));
    auto actionMove8 = MoveBy::create(0.2, Vec2(0, static_cast<float>(y_screen * 0.5)));
    auto seq3 = Sequence::create(actionMove5, actionMove6, nullptr);
    auto seq4 = Sequence::create(actionMove7, actionMove8, nullptr);
    auto seq_callback = Sequence::create(fadeout->clone(), callback, nullptr);
    menu_surclassement_txt->runAction(seq);
    menu_surclassement_img->runAction(seq->clone());
    menu_shop_img->runAction(seq3);
    menu_title->runAction(seq4);
    options_btn->runAction(seq4->clone());
    menu_best_img->runAction(fadeout);
    menu_best_txt->runAction(fadeout->clone());
    menu_anim_img->runAction(fadeout->clone());
    menu_anim_img_hand->runAction(fadeout->clone());
    menu_power_level->runAction(fadeout->clone());
    menu_speed_level->runAction(fadeout->clone());
    menu_stats_img->runAction(seq_callback);
}

int GameScene::get_next_line_id(int type) {
    int *range = Utils::get_container_range_research(type);
    for (int i = range[0]; i <= range[1]; i++) {
        if (pool_container[i]->get_type() == type && !pool_container[i]->line_active)
            return (i);
    }
    return -1;
}

bool GameScene::point_into_square(Square *sq, Vec2 bullet_pos) {
    Vec2 sq_top_left;
    float sq_width = sq->getContentSize().width;
    float sq_height = sq->getContentSize().height;
    sq_top_left.x = sq->getPositionX() - sq_width / 2;
    sq_top_left.y = sq->getPositionY() + sq_height / 2;

    return bullet_pos.x >= sq_top_left.x &&
           bullet_pos.x <= sq_top_left.x + sq_width &&
           bullet_pos.y <= sq_top_left.y &&
           bullet_pos.y >= sq_top_left.y - sq_height;
}

void GameScene::show_destruction_circle(Vec2 pos) {
    for (int i = 0; pool_circle[i]; i++) {
        if (!pool_circle[i]->active_circle) {
            pool_circle[i]->anim_circle(pos);
            return;
        }
    }
}

int GameScene::get_line_in_contact(int bullet_id) {
    float bullet_y = bullet_container[bullet_id]->getPositionY();
    for (int i = 0; active_lines[i] != '\0'; i++) {
        if (active_lines[i] != EMPTY_VALUE) {
            float line_bottom = pool_container[active_lines[i]]->getPositionY();
            float line_top = line_bottom + pool_container[active_lines[i]]->getContentSize().height;
            if (bullet_y >= line_bottom && bullet_y <= line_top)
                return (active_lines[i]);
        }
    }
    return (-1);
}

void GameScene::show_particle(Vec2 pos, Square *sq) {
    if (!sq || !sq->isVisible() || sq->particle_played == 2)
        return;
    sq->particle_played++;
    auto fileUtil = FileUtils::getInstance();
    auto plistData = fileUtil->getValueMapFromFile(PARTICLE_ANIM_PLIST);
    auto stars = ParticleSystemQuad::create(plistData);
    auto delay = DelayTime::create(0.2f);
    auto callback = CallFuncN::create(
            [&](Node *sender) {
                Square *s = ((Square *) sender);
                s->particle_played--;
            });
    auto sequence = Sequence::create(delay, callback, nullptr);
    stars->setDuration(0.2f);
    stars->setPosition(pos);
    stars->setTotalParticles(10);
    stars->setAutoRemoveOnFinish(true);
    runAction(sequence);
    if (stars)
        addChild(stars, 1, 1);
}

void GameScene::show_particle_explode(Vec2 square_pos, int default_color_code, int extra) {
    auto fileUtil = FileUtils::getInstance();
    auto plistData = fileUtil->getValueMapFromFile(explode_plist[RED]);
    switch (default_color_code) {
        case RED:
            plistData = fileUtil->getValueMapFromFile(explode_plist[RED]);
            break;
        case ORANGE:
            plistData = fileUtil->getValueMapFromFile(explode_plist[ORANGE]);
            break;
        case YELLOW:
            plistData = fileUtil->getValueMapFromFile(explode_plist[YELLOW]);
            break;
        case GREEN:
            plistData = fileUtil->getValueMapFromFile(explode_plist[GREEN]);
            break;
        default:
            plistData = fileUtil->getValueMapFromFile(explode_plist[GREEN]);
            break;
    }
    ParticleSystemQuad *stars = ParticleSystemQuad::create(plistData);
    if (!stars)
        return;
    stars->setScale(0.7);
    if (extra == MAX_PARTICLE) {
        stars->setDuration(0.5f);
        stars->setPosition(square_pos);
        stars->setTotalParticles(80);
    } else {
        stars->setDuration(0.5f);
        stars->setPosition(square_pos);
        stars->setTotalParticles(20);
    }
    stars->setAutoRemoveOnFinish(true);
    addChild(stars, 2);
}

void GameScene::show_bonus_particle_explode(Vec2 bonus_pos) {
    auto fileUtil = FileUtils::getInstance();
    auto plistData = fileUtil->getValueMapFromFile(explode_bonus_plist[bonus_active]);
    ParticleSystemQuad *stars = ParticleSystemQuad::create(plistData);
    stars->setScale(1);
    stars->setDuration(0.4);
    stars->setPosition(bonus_pos);
    stars->setTotalParticles(10);
    stars->setAutoRemoveOnFinish(true);
    addChild(stars, 3, 1);
}

void GameScene::check_hit_color_change(Line *l, Square *sq) {
    int default_color_code = sq->initial_color;
    int default_pv_value = sq->initial_pv;
    int pv = sq->square_pv;
    int step = 0;

    auto batch = l->getChildByTag(LINE_BATCH_ID);
    Sprite *e = ((Sprite *) batch->getChildByTag(sq->getTag()));

    if (pv < default_pv_value && pv >= default_pv_value * (0.8))
        step = 0;
    else if (pv >= default_pv_value * 0.6)
        step = 1;
    else if (pv >= default_pv_value * 0.4)
        step = 2;
    else if (pv >= default_pv_value * 0.2)
        step = 3;
    else
        step = 4;
    SpriteFrame *tmp = (SpriteFrame *) COLOR_HIT[default_color_code][step];
    if (e->getSpriteFrame() != tmp)
        e->setSpriteFrame(COLOR_HIT[default_color_code][step]);
}

void GameScene::destroy_all_lines() {
    for (int i = 0; active_lines[i]; i++) {
        if (active_lines[i] != EMPTY_VALUE && pool_container[active_lines[i]]->line_active)
            destroy_complete_line(active_lines[i], pool_container[active_lines[i]]->getPositionY());
    }
}

void GameScene::destroy_complete_line(int line_id, float line_y) {
    if (!pool_container[line_id]->line_active)
        return;
    int i = 0;
    auto batch = pool_container[line_id]->getChildByTag(LINE_BATCH_ID);
    while (1) {
        auto child = pool_container[line_id]->getChildByTag(i);
        if (!child)
            break;
        Square *sq = ((Square *) child);
        auto sprite = batch->getChildByTag(sq->getTag());
        show_destruction_circle(Vec2(sq->getPositionX(), line_y));
        show_particle_explode(
                Vec2(sq->getPositionX(), line_y - sq->getContentSize().height),
                sq->initial_color, NORMAL_PARTICLE);
        sq->setVisible(false);
        sprite->setVisible(false);
        i++;
    }
    if (star_bonus_active) {
        star_bonus_active = false;
        star_line_id = -1;
    }
}

void GameScene::show_destruction_bonus(int value, int line_id) {
    if (!pool_container[line_id] || !pool_container[line_id]->line_active)
        return;
    pool_container[line_id]->half_animated = 1;
    Label *bonus = Label::createWithTTF(Utils::get_reduced_value(value, VALUE_BONUS_COMMENT),
                                        FIRE_UP_FONT_NUMBERS, 40);
    bonus->setAnchorPoint(Vec2(0, 0));
    bonus->setPosition(
            Vec2(x_screen / 2 - bonus->getContentSize().width / 2, y_screen / 2 + y_screen / 12));
    auto move = MoveBy::create(1.0f, Vec2(0, y_screen / 16));
    bonus->setHorizontalAlignment(TextHAlignment::CENTER);
    bonus->setOpacity(0);
    auto actionRemove = RemoveSelf::create();
    auto fadeout = FadeTo::create(1.0f, 10);
    auto fadein = FadeIn::create(0.1f);
    auto delay = DelayTime::create(0.1f);
    auto sequence = Sequence::create(delay, move, actionRemove, nullptr);
    auto sequence_fadeout = Sequence::create(delay, fadeout, nullptr);
    addChild(bonus);
    bonus->runAction(fadein);
    bonus->runAction(sequence_fadeout);
    bonus->runAction(sequence);
}

void GameScene::check_full_destruction_bonus(Line *l, int line_id) {
    if (!l || !l->line_active || l->square_nbr >= 15)
        return;
    if (l->half_animated == 1)
        return;
    int index = 0;
    int square_dead = 0;
    while (1) {
        Square *sq = ((Square *) l->getChildByTag(index));
        if (!sq)
            break;
        if (!sq->isVisible())
            square_dead++;
        index++;
    }
    if (square_dead == l->square_nbr) {
        update_game_score(l->half_total);
        show_destruction_bonus(l->half_total, line_id);
    }
}

void GameScene::check_bullet_collision() {
    for (int i = 0; bullet_container[i] != NULL; i++) {
        int into_line_id = get_line_in_contact(i);
        if (bullet_container[i] && bullet_container[i]->bullet_active && into_line_id != -1)
            check_into_line(i, into_line_id);
    }
}

void GameScene::check_into_line(int bullet_id, int line_id) {
    int index = 0;
    Line *current_line = pool_container[line_id];
    auto batch = current_line->getChildByTag(LINE_BATCH_ID);
    while (1) {
        Square *sq = ((Square *) current_line->getChildByTag(index));
        if (!sq)
            break;
        Vec2 bullet_pos;
        bullet_pos.x = bullet_container[bullet_id]->getPositionX();
        bullet_pos.y = bullet_container[bullet_id]->getPositionY() -
                       (current_line->getPositionY());
        if (bullet_pos.y < 0)
            bullet_pos.y = 0;
        if (sq->isVisible() && point_into_square(sq, bullet_pos)) {
            int bullet_hit = UserLocalStore::get_achievement_variable(POWER_VALUE);
            if (game_shooter_type == POWER_TANK)
                bullet_hit *= 1.5;
            if (bonus_active == BONUS_POWER)
                bullet_hit *= 2;
            if (sq->get_square_pv() - bullet_hit <= 1) {
                auto sprite = batch->getChildByTag(sq->getTag());
                sprite->setVisible(false);
                sq->setVisible(false);
                Vec2 square_pos;
                square_pos.x = sq->getPositionX();
                square_pos.y = current_line->getPositionY() + sq->getPositionY();
                update_game_score(sq->square_pv);
                play_square_explode();
                if (sq->star_bonus == 1) {
                    destroy_all_lines();
                    game_block_destroyed++;
                    bullet_container[bullet_id]->reset();
                    return;
                }
                check_full_destruction_bonus(current_line, line_id);
                show_destruction_circle(square_pos);
                show_particle_explode(
                        Vec2(square_pos.x, square_pos.y - sq->getContentSize().height),
                        sq->initial_color, NORMAL_PARTICLE);
                game_block_destroyed++;
            } else {
                update_game_score(bullet_hit);
                play_bullet_impact();
                show_particle(bullet_container[bullet_id]->getPosition(), sq);
                sq->square_pv -= bullet_hit;
                if (current_line->get_type() > LINE_TYPE_STARTUP_5)
                    check_hit_color_change(current_line, sq);
                int sq_points_value = sq->square_pv;
                sq->points->setString(
                        Utils::get_reduced_value(sq_points_value, VALUE_SIMPLE));
            }
            bullet_container[bullet_id]->reset();
            break;
        }
        index++;
    }
}

void GameScene::check_lines_out() {
    for (int i = 0; active_lines[i] != '\0'; i++) {
        if (active_lines[i] != EMPTY_VALUE && pool_container[active_lines[i]]->getPositionY() +
                                              pool_container[active_lines[i]]->getContentSize().height <=
                                              0) {
            if (pool_container[active_lines[i]]->getScale() == 0.85f) {
                pool_container[active_lines[i]]->setScale(1);
                pool_container[active_lines[i]]->setAnchorPoint(Vec2(0, 0));
            }
            if (active_lines[i] == star_line_id) {
                star_line_id = -1;
                star_bonus_active = false;
            }
            pool_container[active_lines[i]]->reset();
            remove_active_line(i);
        }
    }
}

void GameScene::move_active_lines() {
    for (int i = 0; active_lines[i] != '\0'; i++) {
        if (active_lines[i] != EMPTY_VALUE)
            pool_container[active_lines[i]]->setPositionY(
                    pool_container[active_lines[i]]->getPositionY() - LINE_SPEED);
    }
}

void GameScene::check_player_collision() {
    for (int i = 0; active_lines[i] != '\0'; i++) {
        if (active_lines[i] != -1 && pool_container[active_lines[i]]->getPositionY() <
                                     player->getPositionY() +
                                     (1.1 * player->getContentSize().height / 2)) {
            Vec2 player_pos = player->getPosition();
            int index = 0;
            float player_y = player_pos.y - pool_container[active_lines[i]]->getPositionY();
            while (1) {
                auto child = pool_container[active_lines[i]]->getChildByTag(index);
                Square *sq = ((Square *) child);
                if (!child)
                    break;
                float sq_width = sq->getContentSize().width;
                float sq_height = sq->getContentSize().height;
                float player_width = player->getContentSize().width;
                float player_height = player->getContentSize().height;
                Vec2 player_left_top_corner = Vec2(player_pos.x - player_width / 2,
                                                   player_y + player_height / 2);

                Rect player_full_rect = Rect(player_pos.x - player_width / 2,
                                             player_y + player_height / 2,
                                             player_width, player_height);
                Rect sq_bounding_box = Rect(sq->getPositionX() - sq_width / 2,
                                            sq->getPositionY() + sq_height / 2, sq_width,
                                            sq_height);
                Rect player_crash_bounding_box = Rect(
                        static_cast<float>(player_left_top_corner.x + (0.15 * player_width)),
                        player_left_top_corner.y,
                        static_cast<float>(0.7 * player_width), player_height);

                Rect player_collision_left = Rect(player_left_top_corner.x,
                                                  player_left_top_corner.y,
                                                  static_cast<float>(0.15 * player_width),
                                                  player_height);
                Rect player_collision_right = Rect(
                        static_cast<float>(player_left_top_corner.x + player_width * 0.85),
                        player_left_top_corner.y,
                        static_cast<float>(0.15 * player_width), player_height);

                if (sq->isVisible() && sq_bounding_box.intersectsRect(player_full_rect)) {
                    if (sq->isVisible() &&
                        sq_bounding_box.intersectsRect(player_crash_bounding_box)) {
                        last_line_hited_id = active_lines[i];
                        end_of_game();
                    } else if (sq->isVisible() && sq_bounding_box.intersectsRect(
                            player_collision_left)) {
                        player->setPositionX(player->getPositionX() + player_width / 4);
                    } else if (sq->isVisible() && sq_bounding_box.intersectsRect(
                            player_collision_right)) {
                        player->setPositionX(player->getPositionX() - player_width / 4);
                    }
                    if (player->getPositionX() - player_width / 2 < 0 ||
                        player->getPositionX() + player_width / 2 > x_screen ||
                        player->getPositionY() + player_height / 2 > y_screen ||
                        player->getPositionY() - player_height / 2 < 0)
                        end_of_game();
                }
                index++;
            }
        }
    }
}

void GameScene::move_circles() {
    for (int i = 0; pool_circle[i]; i++) {
        if (pool_circle[i]->active_circle)
            pool_circle[i]->setPositionY(pool_circle[i]->getPositionY() - LINE_SPEED);
    }
}

void GameScene::load_bonus() {
    int type = Utils::get_random_number(0, 2);
    Vec2 initial_pos = Vec2(
            Utils::get_random_number(
                    static_cast<int>(bonus_container[BONUS_BULLET]->getContentSize().width / 2),
                    static_cast<int>(x_screen -
                                     bonus_container[BONUS_BULLET]->getContentSize().width /
                                     2)),
            y_screen + bonus_container[BONUS_BULLET]->getContentSize().height / 2);
    bonus_container[type]->setTag(BONUS_HIDE);
    bonus_container[type]->setPosition(initial_pos);
    bonus_id = type;
    bonus_selected = true;
}

void GameScene::active_bonus() {
    bonus_container[bonus_id]->setTag(BONUS_IN_GAME);
    if (LINE_GENERATED > TRANSITION_FROM_4_TO_5)
        bonus_container[bonus_id]->setScale(1);
    else
        bonus_container[bonus_id]->setScale(1.2f);
    bonus_container[bonus_id]->setVisible(true);
    bonus_container[bonus_id]->runAction(Utils::get_bonus_animation(bonus_id));
    bonus_displayed = true;
    next_bonus_spawn = LINE_GENERATED + 5 + Utils::get_random_number(0, 5);
}

void GameScene::remove_bonus() {
    float initial_y_pos = y_screen + bonus_container[BONUS_BULLET]->getContentSize().height / 2;
    if (bonus_id >= 0 && bonus_id <= 2) {
        bonus_container[bonus_id]->setTag(BONUS_HIDE);
        bonus_container[bonus_id]->stopAllActions();
        bonus_container[bonus_id]->setVisible(false);
        bonus_container[bonus_id]->setPositionY(initial_y_pos);
        rect_container[bonus_id]->setVisible(false);
        rect_container[bonus_id]->stopAllActions();
    }
    bonus_id = -1;
    bonus_active = -1;
    bonus_time = 0;
    bonus_selected = false;
    bonus_displayed = false;
    rect_animated = false;
}

void GameScene::move_bonus() {
    Vec2 player_pos = player->getPosition();
    if (bonus_active != -1) {
        rect_container[bonus_id]->setPosition(player->getPosition());
    } else {
        bonus_container[bonus_id]->setPositionY(
                bonus_container[bonus_id]->getPositionY() - LINE_SPEED);
        if (bonus_container[bonus_id]->getPositionY() +
            bonus_container[bonus_id]->getContentSize().height / 2 <= 0)
            remove_bonus();
    }
}

void GameScene::bonus_collision() {
    float player_width = player->getContentSize().width;
    float player_height = player->getContentSize().height;
    Rect player_bounding_box = Rect(player->getPositionX() - player_width / 2,
                                    player->getPositionY() + player_height / 2, player_width,
                                    player_height);
    float bonus_width = bonus_container[bonus_id]->getContentSize().width;
    float bonus_height = bonus_container[bonus_id]->getContentSize().height;

    Rect bonus_bounding_box = Rect(
            bonus_container[bonus_id]->getPositionX() - bonus_width / 2,
            bonus_container[bonus_id]->getPositionY() + bonus_height / 2, bonus_width,
            bonus_height);
    if (player_bounding_box.intersectsRect(bonus_bounding_box)) {
        float initial_y_pos = y_screen + bonus_container[BONUS_BULLET]->getContentSize().height / 2;
        if (bonus_id == BONUS_SPEED) {
            stop_bullet_shoot();
            bonus_active = bonus_id;
            start_bullet_shoot();
        } else if (bonus_id == BONUS_POWER) {
            bonus_active = bonus_id;
            game_power_up_collected++;
        } else {
            bonus_active = bonus_id;
        }
        bonus_container[bonus_id]->setTexture(default_bonus_texture[bonus_id]);
        bonus_container[bonus_id]->setVisible(false);
        rect_container[bonus_id]->setVisible(true);
        rect_container[bonus_id]->setScale(player->getScale());
        rect_container[bonus_id]->setPosition(player->getPosition());
        show_destruction_circle(bonus_container[bonus_id]->getPosition());
        show_bonus_particle_explode(Vec2(bonus_container[bonus_id]->getPosition().x,
                                         bonus_container[bonus_id]->getPosition().y -
                                         bonus_container[bonus_id]->getContentSize().width));
        bonus_container[bonus_id]->setPositionY(initial_y_pos);
    }
}

void GameScene::bonus_managment() {
    if (LINE_GENERATED == next_bonus_spawn && !bonus_selected)
        load_bonus();
    if (bonus_id != -1 && !bonus_displayed &&
        (pool_container[CURRENT_LINE_ID]->getPosition().y +
         pool_container[CURRENT_LINE_ID]->getContentSize().height) <=
        (NEW_SPAWN_Y + ((y_screen - NEW_SPAWN_Y) / 2)))
        active_bonus();
    if (bonus_displayed) {
        bonus_collision();
        move_bonus();
        if (bonus_active != -1) {
            if (bonus_time >= BONUS_TIME_MIDLE && !rect_animated) {
                rect_animated = true;
                rect_container[bonus_id]->runAction(Utils::get_blink_animation());
            } else if (bonus_time >= BONUS_TIME_LIMIT) {
                rect_container[bonus_id]->setOpacity(100);
                if (bonus_active == BONUS_SPEED) {
                    stop_bullet_shoot();
                    remove_bonus();
                    start_bullet_shoot();
                    return;
                }
                remove_bonus();
            }
        }
    }
}

void GameScene::scale_animation() {
    auto scaleto = ScaleTo::create(1.3, 0.85f);
    player->runAction(scaleto);
    pool_container[CURRENT_LINE_ID]->setAnchorPoint(Vec2(0.5, 0.5));
    pool_container[CURRENT_LINE_ID]->setPositionX(x_screen / 2);
    pool_container[CURRENT_LINE_ID]->runAction(scaleto->clone());
}

void GameScene::generate_star_bonus() {
    if (pool_container[NEXT_LINE_ID]->get_type() == LINE_TYPE_SIMPLE_OF_5 &&
        Utils::get_random_number(0, 5) == 5) {
        pool_container[NEXT_LINE_ID]->attach_star_bonus();
        star_bonus_active = true;
        star_line_id = NEXT_LINE_ID;
    }
}

void GameScene::update(float ft) {
    game_duration += ft;
    if (bonus_active != -1)
        bonus_time += ft;
    if (game_shooter_type == SHIELD_TANK && shield_rect->isVisible())
        shield_rect->setPosition(player->getPosition());
    check_lines_out();
    move_active_lines();
    if (game_state == GAME_RUNNING) {
        if (LINE_GENERATED == TRANSITION_FROM_4_TO_5 && shooter_never_updated == 0 &&
            player->getScale() != 0.85)
            scale_animation();
        move_circles();
        bonus_managment();
        check_bullet_collision();
        check_player_collision();
    }
    if (pool_container[CURRENT_LINE_ID]->getPosition().y <= NEW_SPAWN_Y) {
        if (!star_bonus_active)
            generate_star_bonus();
        pool_container[NEXT_LINE_ID]->set_active(current_factor_h, LINE_GENERATED);
        store_active_line(NEXT_LINE_ID);
        CURRENT_LINE_ID = NEXT_LINE_ID;
        NEXT_LINE_ID = get_next_line_id(get_next_line_type());
        LINE_GENERATED++;
        if (NEXT_LINE_ID != -1)
            NEW_SPAWN_Y = Utils::get_spawn_y(pool_container[CURRENT_LINE_ID]->get_type(),
                                             pool_container[NEXT_LINE_ID]->get_type(),
                                             pool_container[NEXT_LINE_ID]->line_size);
    }
    if (NEXT_LINE_ID == -1) {
        NEXT_LINE_ID = get_next_line_id(get_next_line_type());
        if (NEXT_LINE_ID != -1)
            NEW_SPAWN_Y = Utils::get_spawn_y(pool_container[CURRENT_LINE_ID]->get_type(),
                                             pool_container[NEXT_LINE_ID]->get_type(),
                                             pool_container[NEXT_LINE_ID]->line_size);
    }
}

void GameScene::store_active_line(int line_index) {
    for (int i = 0; active_lines[i] != '\0'; i++) {
        if (active_lines[i] == EMPTY_VALUE) {
            active_lines[i] = line_index;
            return;
        }
    }
}

void GameScene::remove_active_line(int line_to_rm) {
    active_lines[line_to_rm] = EMPTY_VALUE;
}

int GameScene::get_next_line_type() {
    if (shooter_never_updated)
        return (LINE_TYPE_SIMPLE_OF_4);
    if (LINE_GENERATED < SIMPLE_LINE_NBR)
        return (LINE_TYPE_SIMPLE_OF_4);
    else if (LINE_GENERATED == SIMPLE_LINE_NBR)
        return (LINE_TYPE_SIMPLE_OF_5);
    else
        return (Utils::get_random_line_type());
}

void GameScene::run_game_loop() {
    int indicator = UserLocalStore::get_achievement_variable(POWER_LEVEL) +
                    UserLocalStore::get_achievement_variable(SPEED_LEVEL);
    if (indicator == NO_SHOOTER_UPGRADE)
        shooter_never_updated = 1;
    else
        shooter_never_updated = 0;
    if (shooter_never_updated == 1) {
        CURRENT_LINE_ID = 4;
        NEXT_LINE_ID = 5;
        NEW_SPAWN_Y = Utils::get_spawn_y(pool_container[CURRENT_LINE_ID]->get_type(),
                                         pool_container[NEXT_LINE_ID]->get_type(),
                                         pool_container[NEXT_LINE_ID]->line_size);
    } else {
        if (current_factor_h < STARTUP_OF_3) {
            CURRENT_LINE_ID = 1;
            NEXT_LINE_ID = 4;
        } else if (current_factor_h < STARTUP_OF_4) {
            CURRENT_LINE_ID = 2;
            NEXT_LINE_ID = 4;
        } else {
            CURRENT_LINE_ID = 3;
            NEXT_LINE_ID = 4;
        }
        NEW_SPAWN_Y = Utils::get_spawn_y(pool_container[CURRENT_LINE_ID]->get_type(),
                                         pool_container[NEXT_LINE_ID]->get_type(),
                                         pool_container[CURRENT_LINE_ID]->line_size);
    }
    bonus_selected = false;
    next_bonus_spawn = MIN_LINE_BEFORE_BONUS_SPAWN + Utils::get_random_number(0, 5);
    store_active_line(CURRENT_LINE_ID);
    pool_container[CURRENT_LINE_ID]->set_active(
            current_factor_h, LINE_GENERATED);
    LINE_GENERATED++;
    this->scheduleUpdate();
}

void GameScene::stop_game_loop() {
    this->unscheduleUpdate();
}

void GameScene::score_animation() {
    if (score->getNumberOfRunningActions() > 0)
        return;
    auto scale_up = ScaleTo::create(0.04, 1.15f);
    auto scale_down = ScaleTo::create(0.04, 1.0f);
    auto sequence = Sequence::create(scale_up, scale_down, nullptr);
    score->runAction(sequence);
    if (best_img->isVisible())
        best_img->runAction(sequence->clone());
}

void GameScene::update_game_score(int points) {
    int best = UserLocalStore::get_achievement_variable(SCORE);
    score_animation();
    game_score += points;
    if (!best_img->isVisible() && game_score > best) {
        best_img->setVisible(true);
        auto move = MoveTo::create(0.2, Vec2(x_screen / 2, best_img->getPositionY()));
        best_img->runAction(move);
    }
    score->setString(Utils::get_reduced_value(game_score, VALUE_SIMPLE));
}

float GameScene::get_shoot_interval() {
    float factor = 5 * UserLocalStore::get_achievement_variable_float(SPEED_VALUE);
    return static_cast<float>(1.0 / factor);
}

int GameScene::get_h_value() {
    return static_cast<int>(
            (DEFAULT_BULLET_NBR * UserLocalStore::get_achievement_variable_float(SPEED_VALUE)) *
            UserLocalStore::get_achievement_variable(POWER_VALUE));
}

void GameScene::start_game() {
    sdkbox::PluginAdMob::cache(CONTINUE_REWARD_AD_TEST);
    sdkbox::PluginAdMob::cache(TEST_AD);
    //sdkbox::PluginAdMob::cache(BONUSX2_AD_TEST);
    game_already_resumed = false;
    game_shooter_type = Utils::get_shooter_type(UserLocalStore::get_current_shooter());
    if (game_shooter_type == SHIELD_TANK) {
        shield_rect->setVisible(true);
        shield_rect->setPosition(player->getPosition());
    }
    if (options_state == OPTIONS_DISPLAYED)
        options(nullptr);
    game_score = 0;
    bonus_active = -1;
    current_factor_h = get_h_value();
    char score_value[DEFAULT_CHAR_LENGHT];
    sprintf(score_value, "%i", game_score);
    game_state = GAME_RUNNING;
    score->setVisible(true);
    score->setString(score_value);
    run_start_animation();
    start_bullet_shoot();
    run_game_loop();
    game_duration = 0;
}

void GameScene::resume_game() {
    game_already_resumed = true;
    removeChild(continue_button);
    removeChild(next_button);
    destroy_all_lines();
    if (game_shooter_type == SHIELD_TANK && !shield_live_used)
        shield_rect->setVisible(true);
    if (LINE_GENERATED > TRANSITION_FROM_4_TO_5)
        player->setScale(0.85f);
    score->setVisible(true);
    player->setVisible(true);
    player->setPositionX(x_screen / 2);
    if (game_score >= UserLocalStore::get_achievement_variable(SCORE)) {
        best_img->setVisible(true);
        best_img->setPosition(Vec2(x_screen / 2, best_img->getPositionY()));
    }
    game_state = GAME_RUNNING;
    start_bullet_shoot();
}

void GameScene::shop(cocos2d::Ref *pSender) {
    if (game_state != MENU)
        return;
    auto shop = ShopScene::createScene();
    Director::getInstance()->replaceScene(shop);
}

void GameScene::back_to_menu(cocos2d::Ref *pSender) {
    auto callback = CallFuncN::create(
            [&](Node *sender) {
                UserLocalStore::update_achievements(
                        Utils::get_shooter_type(UserLocalStore::get_current_shooter()),
                        game_block_destroyed, game_power_up_collected);
                game_block_destroyed = 0;
                game_power_up_collected = 0;
                LINE_GENERATED = 0;
                if (!player->isVisible())
                    player->setVisible(true);
                player->setPositionX(x_screen / 2);
                stop_game_loop();
                reset_arrays();
                game_state = MENU;
                removeChild(end_menu);
                game_menu = get_main_menu();
                addChild(game_menu);
                main_menu_coming_animation();
            });
    auto delay = DelayTime::create(0.2);
    auto delay2 = DelayTime::create(0.2);
    auto move_to_share_0 = MoveTo::create(0.1,
                                          Vec2(share->getPositionX(), share->getPositionY() +
                                                                      share->getContentSize().height));
    auto move_to_rate_0 = MoveTo::create(0.1, Vec2(rate->getPositionX(), rate->getPositionY() +
                                                                         rate->getContentSize().height));
    auto move_to_share_1 = MoveTo::create(0.1, Vec2(share->getPositionX(),
                                                    0 - share->getContentSize().height));
    auto move_to_rate_1 = MoveTo::create(0.1, Vec2(rate->getPositionX(),
                                                   0 - rate->getContentSize().height));
    auto move_to_stats = MoveTo::create(0.1, Vec2(stats->getPositionX(),
                                                  y_screen + stats->getContentSize().height));
    auto move_to_back = MoveTo::create(0.1, Vec2(back_to_main->getPositionX(),
                                                 0 - back_to_main->getContentSize().height));

    auto sequence_share = Sequence::create(move_to_share_0, move_to_share_1, nullptr);
    auto sequence_rate = Sequence::create(move_to_rate_0, move_to_rate_1, nullptr);
    auto sequence_stats = Sequence::create(delay, move_to_stats, nullptr);
    auto sequence_back = Sequence::create(delay2, move_to_back, callback, nullptr);
    share->runAction(sequence_share);
    rate->runAction(sequence_rate);
    stats->runAction(sequence_stats);
    back_to_main->runAction(sequence_back);
    if (game_duration >= GAME_DURATION_LIMIT_FOR_ADS)
        sdkbox::PluginAdMob::show(TEST_AD);
}

void GameScene::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event) {
    if (touch->getLocation().x > 0 + player->getContentSize().width / 2 &&
        touch->getLocation().x < x_screen - player->getContentSize().height / 2) {
        if (game_state == MENU)
            return;
        float current_location = touch->getLocation().x;
        float future_x = current_location +
                         Utils::get_finger_move_factor(current_location, game_shooter_type);
        player->setPositionX(future_x);
        if (player->getPositionX() - player->getContentSize().width / 2 < 0)
            player->setPositionX(player->getContentSize().width / 2);
        if (player->getPositionX() + player->getContentSize().width / 2 > x_screen)
            player->setPositionX(x_screen - player->getContentSize().width / 2);
    }
}

void GameScene::manage_options() {
    if (UserLocalStore::get_achievement_variable(SOUND) == SOUND_ON) {
        UserLocalStore::store_achievement_variable(SOUND, SOUND_OFF);
        sound->setTexture(OPTIONS_SOUND_ON);
        sound_activated = false;
    } else {
        UserLocalStore::store_achievement_variable(SOUND, SOUND_ON);
        sound->setTexture(OPTIONS_SOUND_OFF);
        sound_activated = true;
    }
}

bool GameScene::is_touch_on_player_zone(Vec2 touch_position) {
    Vec2 player_pos = player->getPosition();
    Size player_size = player->getContentSize();
    float gap_x = player_size.width / 2 + (2 * player_size.width);
    float gap_y = player_size.height / 2 + (2 * player_size.width);

    return touch_position.x >= player_pos.x - gap_x &&
           touch_position.x <= player_pos.x + gap_x &&
           touch_position.y <= player_pos.y + gap_y && player_pos.y >= player_pos.y - gap_y;
}

bool GameScene::is_next_button_touched(Vec2 touch_location) {
    float x_pos = next_button->getPosition().x - next_button->getContentSize().width / 2;
    float y_pos = next_button->getPosition().y - next_button->getContentSize().height / 2;
    return touch_location.x >= x_pos &&
           touch_location.x <= x_pos + next_button->getContentSize().width &&
           touch_location.y >= y_pos &&
           touch_location.y <= y_pos + next_button->getContentSize().height;
}

bool GameScene::is_continue_button_touched(Vec2 touch_location) {
    float x_pos = continue_button->getPosition().x - continue_button->getContentSize().width / 2;
    float y_pos = continue_button->getPosition().y - continue_button->getContentSize().height / 2;
    return touch_location.x >= x_pos &&
           touch_location.x <= x_pos + continue_button->getContentSize().width &&
           touch_location.y >= y_pos &&
           touch_location.y <= y_pos + continue_button->getContentSize().height;
}

bool GameScene::onTouchBegan(Touch *touch, Event *event) {
    if (game_state == GAME_RUNNING && !is_touch_on_player_zone(touch->getLocation()))
        return false;
    if (game_state == RESUME && continue_button &&
        is_continue_button_touched(touch->getLocation())) {
        while (1) {
            if (sdkbox::PluginAdMob::isAvailable(CONTINUE_REWARD_AD_TEST)) {
                stopAction(wait_sequence);
                continue_button->stopAllActions();
                continue_button->setVisible(false);
                next_button->setVisible(false);
                sdkbox::PluginAdMob::show(CONTINUE_REWARD_AD_TEST);
                break;
            }
        }
        return false;
    }
    if (game_state == RESUME && is_next_button_touched(touch->getLocation())) {
        game_state = GAME_END;
        stopAction(wait_sequence);
        continue_button->stopAllActions();
        removeChild(continue_button);
        removeChild(next_button);
        end_menu = get_end_game_menu();
        addChild(end_menu);
    }
    if (options_state == OPTIONS_DISPLAYED && is_sound_button_touched(touch->getLocation())) {
        manage_options();
        return true;
    }
    if (game_state == MENU)
        start_game();
    return true;
}

bool GameScene::is_sound_button_touched(Vec2 touch_location) {
    float x_pos = sound->getPosition().x - sound->getContentSize().width / 2;
    float y_pos = sound->getPosition().y - sound->getContentSize().height / 2;
    return touch_location.x >= x_pos &&
           touch_location.x <= x_pos + sound->getContentSize().width &&
           touch_location.y >= y_pos &&
           touch_location.y <= y_pos + sound->getContentSize().height;
}

void GameScene::play_bullet_sound() {
    if (!sound_activated)
        return;
    if (launch_played != 0) {
        if (launch_played == 1)
            launch_played = 0;
        else
            launch_played++;
        return;
    }
    launch_played++;
    game_audio->playEffect(SOUND_LAUNCH, false, 1.0f, 1.0f, 1.0f);
}

void GameScene::play_bullet_impact() {
    if (!sound_activated)
        return;
    if (hit_played != 0) {
        if (hit_played == 1)
            hit_played = 0;
        else
            hit_played++;
        return;
    }
    hit_played++;
    game_audio->playEffect(SOUND_HITED, false, 1.0f, 1.0f, 1.0f);
}

void GameScene::play_square_explode() {
    if (!sound_activated)
        return;
    game_audio->playEffect(SOUND_EXPLODE, false, 1.0f, 1.0f, 1.0f);
}

void GameScene::launch_bullet(float dt) {
    if (game_shooter_type != DOUBLE_TANK && game_shooter_type != SIDEWAY_TANK &&
        game_shooter_type != TRIPLE_TANK && bonus_active != BONUS_BULLET) {
        for (int i = 0; bullet_container[i] != NULL; i++) {
            if (!bullet_container[i]->bullet_active) {
                bullet_container[i]->launch(bullet_state, player->getPosition(),
                                            player->getContentSize(), NORMAL_LAUNCH);
                play_bullet_sound();
                if (bonus_active == BONUS_POWER)
                    bullet_container[i]->setScale(BIG_BULLET_SIZE);
                if (bullet_state == BULLET_LEFT)
                    bullet_state = BULLET_RIGHT;
                else
                    bullet_state = BULLET_LEFT;
                return;
            }
        }
    } else if (bonus_active == BONUS_BULLET) {
        int bullet_founded = 0;
        for (int i = 0; bullet_container[i] != NULL; i++) {
            if (!bullet_container[i]->bullet_active) {
                bullet_container[i]->launch(bullet_state, player->getPosition(),
                                            player->getContentSize(), bullet_founded);
                play_bullet_sound();
                bullet_founded++;
                if (bullet_founded == 3)
                    return;
            }
        }
        return;
    } else {
        int bullet_founded = 0;
        int bullet_nbr = Utils::get_bullet_nbr(game_shooter_type);
        int *bullet_ids = new int[bullet_nbr + 1];
        for (int i = 0; bullet_founded != bullet_nbr; i++) {
            if (!bullet_container[i]->bullet_active) {
                bullet_ids[bullet_founded] = i;
                bullet_founded++;
            }
        }
        bullet_ids[bullet_founded] = '\0';
        int type = Utils::get_bullet_shoot_index(game_shooter_type);
        for (int i = 0; i < bullet_nbr; i++) {
            bullet_container[bullet_ids[i]]->launch(bullet_state, player->getPosition(),
                                                    player->getContentSize(),
                                                    BULLET_SHOOT[type][i]);
            play_bullet_sound();
            if (bonus_active == BONUS_POWER)
                bullet_container[i]->setScale(BIG_BULLET_SIZE);
        }
        return;
    }
    return;
}

void GameScene::main_menu_coming_animation() {
    menu_stats_img->setOpacity(0);
    menu_anim_img->setOpacity(0);
    menu_anim_img_hand->setOpacity(0);
    options_btn->setOpacity(0);
    menu_best_txt->setOpacity(0);
    menu_best_img->setOpacity(0);

    menu_title->setPositionY(y_screen + menu_title->getContentSize().height);
    menu_surclassement_img->setPositionX(
            x_screen + menu_surclassement_img->getContentSize().width);
    menu_surclassement_txt->setPositionX(menu_surclassement_img->getPosition().x -
                                         menu_surclassement_img->getContentSize().width / 2);


    auto move_title_0 = MoveTo::create(0.1, Vec2(menu_title->getPositionX(),
                                                 static_cast<float>(
                                                         static_cast<float>(y_screen -
                                                                            (y_screen /
                                                                             5)) -
                                                         y_screen * 0.05)));
    auto move_title_1 = MoveBy::create(0.1, Vec2(0, static_cast<float>(y_screen * 0.05)));
    auto move_surclassement_img_0 = MoveTo::create(0.1,
                                                   Vec2(static_cast<float>(x_screen * 0.95 -
                                                                           x_screen * 0.03),
                                                        menu_surclassement_img->getPositionY()));
    auto move_surclassement_img_1 = MoveBy::create(0.1, Vec2(
            static_cast<float>(x_screen * 0.03),
            0));
    auto move_surclassement_txt_0 = MoveTo::create(0.1,
                                                   Vec2(static_cast<float>(x_screen * 0.95 -
                                                                           x_screen * 0.03) -
                                                        menu_surclassement_img->getContentSize().width /
                                                        2,
                                                        menu_surclassement_txt->getPositionY()));

    auto move_shop_img_0 = MoveTo::create(0.1, Vec2(static_cast<float>(static_cast<float>(
                                                                               menu_shop_img->getContentSize().width /
                                                                               2 +
                                                                               x_screen *
                                                                               0.05) +
                                                                       x_screen * 0.03),
                                                    menu_shop_img->getPositionY()));
    auto move_shop_img_1 = MoveBy::create(0.1, Vec2(static_cast<float>(-x_screen * 0.03), 0));
    auto fadeIn = FadeIn::create(0.5);
    auto seq0 = Sequence::create(move_surclassement_img_0, move_surclassement_img_1, NULL);
    auto seq1 = Sequence::create(move_surclassement_txt_0, move_surclassement_img_1->clone(),
                                 NULL);
    auto seq2 = Sequence::create(move_shop_img_0, move_shop_img_1, NULL);
    auto seq3 = Sequence::create(move_title_0, move_title_1, NULL);
    menu_surclassement_img->runAction(seq0);
    menu_surclassement_txt->runAction(seq1);
    menu_shop_img->runAction(seq2);
    menu_title->runAction(seq3);
    menu_stats_img->runAction(fadeIn);
    menu_anim_img->runAction(fadeIn->clone());
    menu_anim_img_hand->runAction(fadeIn->clone());
    options_btn->runAction(fadeIn->clone());
    menu_best_txt->runAction(fadeIn->clone());
    menu_best_img->runAction(fadeIn->clone());
}

Menu *GameScene::get_main_menu() {
    menu_title = MenuItemFont::create(TITLE, nullptr);
    char power_level[DEFAULT_CHAR_LENGHT];
    sprintf(power_level, "%i",
            UserLocalStore::get_achievement_variable(POWER_VALUE));
    char speed_level[DEFAULT_CHAR_LENGHT];
    sprintf(speed_level, "%.1f",
            UserLocalStore::get_achievement_variable_float(SPEED_VALUE));
    char best_score[DEFAULT_CHAR_LENGHT];
    if (UserLocalStore::get_achievement_variable(SCORE) > 1000)
        sprintf(best_score, "%.1fk",
                static_cast<float>(UserLocalStore::get_achievement_variable(SCORE)) / 1000);
    else
        sprintf(best_score, "%i", UserLocalStore::get_achievement_variable(SCORE));
    menu_power_level = MenuItemFont::create(power_level);
    menu_power_level->setTag(POWER_LEVEL_BTN_TAG);
    menu_speed_level = MenuItemFont::create(speed_level);
    menu_speed_level->setTag(SPEED_LEVEL_BTN_TAG);
    menu_surclassement_txt = MenuItemFont::create(SURCLASSEMENT,
                                                  CC_CALLBACK_1(GameScene::surclassement,
                                                                this));
    menu_surclassement_img = MenuItemImage::create(SURCLASSEMENT_UNSELECTED,
                                                   SURCLASSEMENT_SELECTED,
                                                   CC_CALLBACK_1(GameScene::surclassement,
                                                                 this));
    options_btn = MenuItemImage::create(OPTIONS,
                                        OPTIONS,
                                        CC_CALLBACK_1(GameScene::options, this));
    menu_stats_img = MenuItemImage::create(STATS, STATS);
    menu_anim_img_hand = MenuItemImage::create(HAND,
                                               HAND);
    menu_anim_img = MenuItemImage::create(HAND_RAIL, HAND_RAIL);
    menu_best_img = MenuItemImage::create(BEST_IMG, BEST_IMG);
    menu_best_txt = MenuItemFont::create(best_score);
    if (UserLocalStore::get_achievement_variable(NEW_SHOP_ELEMENT) == 0)
        menu_shop_img = MenuItemImage::create(SHOP_UNSELECTED,
                                              SHOP_SELECTED,
                                              CC_CALLBACK_1(GameScene::shop, this));
    else
        menu_shop_img = MenuItemImage::create(SHOP_UNSELECTED_NEW,
                                              SHOP_SELECTED_NEW,
                                              CC_CALLBACK_1(GameScene::shop, this));
    menu_title->setFontNameObj(FIRE_UP_FONT);
    menu_power_level->setFontNameObj(FIRE_UP_FONT);
    menu_power_level->setFontSizeObj(20);
    menu_speed_level->setFontNameObj(FIRE_UP_FONT);
    menu_speed_level->setFontSizeObj(20);
    menu_surclassement_txt->setFontNameObj(FIRE_UP_FONT);
    menu_anim_img_hand->setAnchorPoint(Vec2(0.45, 0.75));
    menu_title->setFontSizeObj(85);
    menu_best_txt->setFontSizeObj(30);
    menu_surclassement_txt->setFontSizeObj(20);
    menu_surclassement_img->setAnchorPoint(Vec2(1, 1));
    menu_surclassement_img->setPosition(
            Vec2(static_cast<float>(x_screen * 0.95),
                 player->getPosition().y +
                 player->getContentSize().height));
    menu_shop_img->setPosition(
            Vec2(static_cast<float>(menu_shop_img->getContentSize().width / 2 +
                                    x_screen * 0.05),
                 player->getPosition().y + player->getContentSize().height / 2));
    menu_surclassement_txt->setPosition(Vec2(menu_surclassement_img->getPosition().x -
                                             menu_surclassement_img->getContentSize().width / 2,
                                             menu_surclassement_img->getPosition().y -
                                             menu_surclassement_img->getContentSize().height /
                                             2));
    menu_anim_img->setPosition(Vec2(static_cast<float>(x_screen * 0.5),
                                    static_cast<float>(y_screen * 0.17)));
    menu_anim_img_hand->setPosition(Vec2(
            static_cast<float>(menu_anim_img->getPosition().x +
                               0.20 * (0.5 * menu_anim_img->getContentSize().width)),
            static_cast<float>(y_screen * 0.17)));
    menu_title->setPosition(Point(x_screen / 2,
                                  y_screen - (y_screen / 5)));
    options_btn->setPosition(
            static_cast<float>(0 + (options_btn->getContentSize().height / 2) +
                               (0.3 * options_btn->getContentSize().height)),
            static_cast<float>(y_screen - (0.3 * options_btn->getContentSize().height +
                                           (options_btn->getContentSize().height / 2))));
    menu_best_txt->setFontNameObj(FIRE_UP_FONT);
    auto midle = x_screen / 2;
    float img_width = menu_best_img->getContentSize().width;
    float txt_width = menu_best_txt->getContentSize().width;
    float both = img_width + txt_width;
    menu_best_img->setPosition(Point(midle - both / 2 + (img_width / 2 - img_width / 8),
                                     static_cast<float>(menu_title->getPosition().y -
                                                        (menu_title->getContentSize().height *
                                                         0.8))));
    menu_best_txt->setPosition(Point(midle + both / 2 - (txt_width / 2 - img_width / 8),
                                     static_cast<float>(menu_title->getPosition().y -
                                                        ((menu_title->getContentSize().height *
                                                          0.8) +
                                                         menu_best_img->getContentSize().height *
                                                         0.16))));
    auto actionMove1 = MoveTo::create(0.6, Vec2(menu_anim_img->getPosition().x -
                                                (menu_anim_img->getContentSize().width / 2 -
                                                 menu_anim_img->getContentSize().width / 6),
                                                menu_anim_img_hand->getPosition().y));
    auto actionMove2 = MoveTo::create(0.6, Vec2(menu_anim_img->getPosition().x +
                                                (menu_anim_img->getContentSize().width / 2 -
                                                 menu_anim_img->getContentSize().width / 6),
                                                menu_anim_img_hand->getPosition().y));
    menu_stats_img->setAnchorPoint(Vec2(0, 0));
    menu_stats_img->setPosition(Vec2(x_screen / 2 + player->getContentSize().width / 2,
                                     menu_shop_img->getPosition().y +
                                     menu_shop_img->getContentSize().height / 2));
    menu_speed_level->setPosition(
            Vec2(menu_stats_img->getPosition().x + menu_stats_img->getContentSize().width / 2,
                 static_cast<float>(menu_stats_img->getPosition().y +
                                    menu_stats_img->getContentSize().height / 2 +
                                    (0.08 * menu_stats_img->getContentSize().height))));
    menu_power_level->setPosition(
            menu_stats_img->getPosition().x + menu_stats_img->getContentSize().width / 2,
            static_cast<float>(menu_stats_img->getPosition().y +
                               (0.27 * menu_stats_img->getContentSize().height)));
    auto seq = Sequence::create(actionMove1, actionMove2, nullptr);
    menu_anim_img_hand->runAction(RepeatForever::create(seq));
    auto main_menu = Menu::create(menu_title, menu_surclassement_img, menu_surclassement_txt,
                                  menu_shop_img, menu_anim_img, menu_anim_img_hand,
                                  menu_best_img,
                                  menu_best_txt, options_btn, menu_stats_img, menu_power_level,
                                  menu_speed_level,
                                  NULL);
    main_menu->setTag(MAIN_MENU_TAG);
    main_menu->setPosition(Point(0, 0));
    return (main_menu);
}

Menu *GameScene::get_end_game_menu() {
    char power_price[DEFAULT_CHAR_LENGHT];
    sprintf(power_price, "%i PTS",
            UserLocalStore::get_achievement_variable(POWER_LEVEL_PRICE));
    float speed_price_value = UserLocalStore::get_achievement_variable_float(SPEED_LEVEL_PRICE);
    float power_price_value = UserLocalStore::get_achievement_variable_float(POWER_LEVEL_PRICE);
    float point_value = UserLocalStore::get_achievement_variable(POINT);
    char current_power[DEFAULT_CHAR_LENGHT];
    sprintf(current_power, "LEVEL %i",
            UserLocalStore::get_achievement_variable(POWER_LEVEL));
    char current_speed[DEFAULT_CHAR_LENGHT];
    sprintf(current_speed, "LEVEL %i",
            UserLocalStore::get_achievement_variable(SPEED_LEVEL));
    stats = Menu::create();
    Sprite *s = Sprite::create(BONUS_X2);
    Label *current_point = Label::createWithTTF(
            Utils::get_reduced_value(point_value, VALUE_WITH_POINT), FIRE_UP_FONT, 60);
    Label *earned_point = Label::createWithTTF(
            Utils::get_reduced_value(game_score, VALUE_WITH_PLUS), FIRE_UP_FONT, 25);
    Label *speed_price_txt = Label::createWithTTF(
            Utils::get_reduced_value(speed_price_value, VALUE_WITH_POINT), FIRE_UP_FONT, 25);
    Label *power_price_txt = Label::createWithTTF(
            Utils::get_reduced_value(power_price_value, VALUE_WITH_POINT), FIRE_UP_FONT, 25);
    Label *speed_info = Label::createWithTTF("SHOOTING SPEED", FIRE_UP_FONT, 24);
    Label *power_info = Label::createWithTTF("SHOOTING POWER", FIRE_UP_FONT, 24);
    Label *speed_current_level = Label::createWithTTF(current_speed, FIRE_UP_FONT, 25);
    Label *power_current_level = Label::createWithTTF(current_power, FIRE_UP_FONT, 25);
    back_to_main = MenuItemImage::create(REPLAY_TEXTURE,
                                         REPLAY_TEXTURE,
                                         CC_CALLBACK_1(GameScene::back_to_menu, this));
    share = MenuItemImage::create
            (SHARE_TEXTURE,
             SHARE_TEXTURE,
             CC_CALLBACK_1(GameScene::back_to_menu, this));
    rate = MenuItemImage::create(RATE_TEXTURE,
                                 RATE_TEXTURE,
                                 CC_CALLBACK_1(GameScene::back_to_menu, this));
    Sprite *background = Sprite::create(BACKGROUND_TEXTURE);
    speed_current_level->setColor(Color3B(124, 184, 255));
    power_current_level->setColor(Color3B(255, 124, 124));
    auto move_to_stats = MoveTo::create(0.2, Vec2(x_screen / 2,
                                                  static_cast<float>(y_screen / 1.75)));
    MenuItemImage *speed_level_btn = MenuItemImage::create(POPUP_MENU_PATH::
                                                           SPEED,
                                                           POPUP_MENU_PATH::SPEED_SELECTED,
                                                           [=](Ref *sender) {
                                                               GameScene::increase_speed(
                                                                       speed_current_level,
                                                                       speed_price_txt,
                                                                       current_point);
                                                           });
    MenuItemImage *power_level_btn = MenuItemImage::create(POPUP_MENU_PATH::
                                                           POWER,
                                                           POPUP_MENU_PATH::POWER_SELECTED,
                                                           [=](Ref *sender) {
                                                               GameScene::increase_power(
                                                                       power_current_level,
                                                                       power_price_txt,
                                                                       current_point);
                                                           });
    MenuItemImage *bonus_x2 = MenuItemImage::create(BONUS_X2,
                                                    BONUS_X2,
                                                    [=](Ref *sender) {

                                                    });
    stats->setPosition(Size(x_screen / 2, static_cast<float>(y_screen +
                                                             stats->getContentSize().height *
                                                             1.3)));
    stats->setContentSize(Size(static_cast<float>(x_screen * 0.88),
                               static_cast<float>(y_screen / 2.5)));
    back_to_main->setPosition(Vec2(x_screen / 2, -2 * back_to_main->getContentSize().height));
    share->setPosition(
            Vec2(x_screen / 2 - back_to_main->getContentSize().width,
                 -2 * back_to_main->getContentSize().height));
    rate->setPosition(x_screen / 2 + back_to_main->getContentSize().width,
                      -2 * back_to_main->getContentSize().height);
    auto move_to_skip = MoveTo::create(0.2, Vec2(x_screen / 2,
                                                 static_cast<float>(
                                                         (static_cast<float>(y_screen /
                                                                             1.75)) -
                                                         stats->getContentSize().height / 2 -
                                                         1.5 *
                                                         back_to_main->getContentSize().height)));
    auto move_to_share_0 = MoveTo::create(0.2,
                                          Vec2(x_screen / 2 - back_to_main->getContentSize().width,
                                               static_cast<float>(static_cast<float>(
                                                                          (static_cast<float>(
                                                                                  y_screen /
                                                                                  1.75)) -
                                                                          stats->getContentSize().height /
                                                                          2 -
                                                                          1.5 *
                                                                          back_to_main->getContentSize().height) +
                                                                  0.5 *
                                                                  back_to_main->getContentSize().height)));
    auto move_to_share_1 = MoveTo::create(0.1, Vec2(x_screen / 2 -
                                                    back_to_main->getContentSize().width,
                                                    static_cast<float>(
                                                            (static_cast<float>(y_screen /
                                                                                1.75)) -
                                                            stats->getContentSize().height / 2 -
                                                            1.5 *
                                                            back_to_main->getContentSize().height)));
    auto move_to_rate_0 = MoveTo::create(0.2,
                                         Vec2(x_screen / 2 + back_to_main->getContentSize().width,
                                              static_cast<float>(static_cast<float>(
                                                                         (static_cast<float>(
                                                                                 y_screen / 1.75)) -
                                                                         stats->getContentSize().height /
                                                                         2 -
                                                                         1.5 *
                                                                         back_to_main->getContentSize().height) +
                                                                 0.5 *
                                                                 back_to_main->getContentSize().height)));
    auto move_to_rate_1 = MoveTo::create(0.1, Vec2(x_screen / 2 +
                                                   back_to_main->getContentSize().width,
                                                   static_cast<float>(
                                                           (static_cast<float>(y_screen /
                                                                               1.75)) -
                                                           stats->getContentSize().height / 2 -
                                                           1.5 *
                                                           back_to_main->getContentSize().height)));
    auto sequence_share = Sequence::create(move_to_share_0, move_to_share_1, nullptr);
    auto sequence_rate = Sequence::create(move_to_rate_0, move_to_rate_1, nullptr);
    speed_level_btn->setPosition(Vec2(0 - stats->getContentSize().width / 4,
                                      static_cast<float>(-(stats->getContentSize().height / 2) +
                                                         speed_level_btn->getContentSize().height *
                                                         0.7)));
    power_level_btn->setPosition(Vec2(0 + stats->getContentSize().width / 4,
                                      static_cast<float>(-(stats->getContentSize().height / 2) +
                                                         power_level_btn->getContentSize().height *
                                                         0.7)));
    current_point->setPosition(Vec2(0, static_cast<float>(stats->getContentSize().height / 2 -
                                                          (1.1 *
                                                           current_point->getContentSize().height))));
    earned_point->setPosition(Vec2(0, static_cast<float>(current_point->getPositionY() - (1.2 *
                                                                                          current_point->getContentSize().height /
                                                                                          2))));
    speed_price_txt->setPosition(Vec2(speed_level_btn->getPosition().x,
                                      speed_level_btn->getPosition().y -
                                      (speed_level_btn->getContentSize().height / 6)));
    power_price_txt->setPosition(Vec2(power_level_btn->getPosition().x,
                                      power_level_btn->getPosition().y -
                                      (power_level_btn->getContentSize().height / 6)));
    speed_current_level->setPosition(Vec2(speed_level_btn->getPosition().x,
                                          static_cast<float>(speed_level_btn->getPosition().y +
                                                             speed_level_btn->getContentSize().height /
                                                             2 * 1.4)));
    power_current_level->setPosition(Vec2(power_level_btn->getPosition().x,
                                          static_cast<float>(power_level_btn->getPosition().y +
                                                             power_level_btn->getContentSize().height /
                                                             2 * 1.4)));
    speed_info->setPosition(Vec2(speed_current_level->getPosition().x,
                                 static_cast<float>(speed_current_level->getPosition().y +
                                                    1.6 *
                                                    speed_current_level->getContentSize().height)));
    power_info->setPosition(Vec2(power_current_level->getPosition().x,
                                 static_cast<float>(power_current_level->getPosition().y +
                                                    1.6 *
                                                    power_current_level->getContentSize().height)));
    background->setContentSize(stats->getContentSize());
    bonus_x2->setAnchorPoint(Vec2(0, 0));
    bonus_x2->setScale(0.7);
    bonus_x2->setPositionY(
            static_cast<float>(bonus_x2->getPositionY() +
                               (0.9 * stats->getContentSize().height / 2)));
    stats->addChild(background);
    stats->addChild(bonus_x2);
    stats->addChild(earned_point);
    stats->addChild(current_point);
    stats->addChild(power_level_btn);
    stats->addChild(speed_level_btn);
    stats->addChild(speed_price_txt);
    stats->addChild(power_price_txt);
    stats->addChild(speed_info);
    stats->addChild(power_info);
    stats->addChild(speed_current_level);
    stats->addChild(power_current_level);
    back_to_main->runAction(move_to_skip);
    share->runAction(sequence_share);
    rate->runAction(sequence_rate);
    stats->runAction(move_to_stats);
    addChild(stats);
    bonus_x2->isIgnoreAnchorPointForPosition();
    auto end_menu = Menu::create(back_to_main, share, rate, NULL);
    end_menu->setAnchorPoint(Vec2(0.5, 0.5));
    end_menu->setPosition(Vec2(0, 0));
    end_menu->setContentSize(Size(x_screen, y_screen));
    return (end_menu);
}

void GameScene::increase_speed(Label *level, Label *price, Label *points) {
    float actual_coin = UserLocalStore::get_achievement_variable(POINT);
    float actual_price = UserLocalStore::get_achievement_variable(SPEED_LEVEL_PRICE);
    if (actual_coin < actual_price)
        return;
    int ex_level = UserLocalStore::get_achievement_variable(SPEED_LEVEL);
    float new_speed_factor = 0.5;
    if (ex_level > 7)
        new_speed_factor = 0.3;
    UserLocalStore::store_achievement_variable_float(SPEED_VALUE,
                                                     UserLocalStore::get_achievement_variable_float(
                                                             SPEED_VALUE) +
                                                     new_speed_factor);
    UserLocalStore::store_achievement_variable(POINT,
                                               static_cast<int>(actual_coin - actual_price));
    if (ex_level + 1 > 4) {
        UserLocalStore::store_achievement_variable_float(SPEED_LEVEL_PRICE,
                                                         static_cast<float>(
                                                                 UserLocalStore::get_achievement_variable_float(
                                                                         SPEED_LEVEL_PRICE) *
                                                                 1.62));
    } else {
        UserLocalStore::store_achievement_variable_float(SPEED_LEVEL_PRICE,
                                                         speed_price[UserLocalStore::get_achievement_variable(
                                                                 SPEED_LEVEL)]);
    }
    UserLocalStore::store_achievement_variable(
            SPEED_LEVEL,
            UserLocalStore::get_achievement_variable(
                    SPEED_LEVEL) +
            1);
    if (UserLocalStore::get_achievement_variable(SPEED_LEVEL) == 10)
        UserLocalStore::store_shooter(20, 1);
    char s[DEFAULT_CHAR_LENGHT];
    sprintf(s, "SPEED %i", UserLocalStore::get_achievement_variable(SPEED_LEVEL));
    level->setString(s);
    float price_value = UserLocalStore::get_achievement_variable(SPEED_LEVEL_PRICE);
    price->setString(Utils::get_reduced_value(price_value, VALUE_WITH_POINT));
    float final_point = UserLocalStore::get_achievement_variable(POINT);
    points->setString(Utils::get_reduced_value(final_point, VALUE_WITH_POINT));
}

void GameScene::increase_power(Label *power, Label *price, Label *points) {
    float actual_coin = UserLocalStore::get_achievement_variable(POINT);
    float actual_price = UserLocalStore::get_achievement_variable(POWER_LEVEL_PRICE);
    if (actual_coin < actual_price)
        return;
    int ex_level = UserLocalStore::get_achievement_variable(POWER_LEVEL);
    UserLocalStore::store_achievement_variable(
            POWER_VALUE,
            UserLocalStore::get_achievement_variable(
                    POWER_VALUE) +
            2);
    UserLocalStore::store_achievement_variable(POINT,
                                               static_cast<int>(actual_coin - actual_price));
    if (ex_level + 1 > 4) {
        UserLocalStore::store_achievement_variable_float(POWER_LEVEL_PRICE,
                                                         static_cast<float>(
                                                                 UserLocalStore::get_achievement_variable_float(
                                                                         POWER_LEVEL_PRICE) *
                                                                 1.62));
    } else {
        UserLocalStore::store_achievement_variable_float(POWER_LEVEL_PRICE,
                                                         power_price[ex_level]);
    }
    UserLocalStore::store_achievement_variable(
            POWER_LEVEL,
            UserLocalStore::get_achievement_variable(
                    POWER_LEVEL) +
            1);
    if (UserLocalStore::get_achievement_variable(POWER_LEVEL) ==
        20)// DEBLOCAGE D'UN SHOOTER (5:)
        UserLocalStore::store_shooter(6, 1);
    char p[DEFAULT_CHAR_LENGHT];
    sprintf(p, "LEVEL %i",
            UserLocalStore::get_achievement_variable(POWER_LEVEL));
    power->setString(p);
    float price_value = UserLocalStore::get_achievement_variable_float(POWER_LEVEL_PRICE);
    price->setString(Utils::get_reduced_value(price_value, VALUE_WITH_POINT));
    float final_point = UserLocalStore::get_achievement_variable(POINT);
    points->setString(Utils::get_reduced_value(final_point, VALUE_WITH_POINT));
}

void GameScene::check_first_launch() {
    if (UserLocalStore::get_first_launch() == 0) {
        UserLocalStore::set_first_launch();
        UserLocalStore::init_asset_arrays();
        UserLocalStore::init_achievement_variables();
    }
}

void GameScene::menuCloseCallback(Ref *pSender) {
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}