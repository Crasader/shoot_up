#include        <string>
#include        <random>
#include        "cocos2d.h"
#include        "Utils.h"
#include        "GameScene.h"
#include        "UserLocalStore.h"
#include        "ShootUp.h"

using namespace std;

USING_NS_CC;

Action *Utils::get_blink_animation() {
    auto fadeout = FadeOut::create(0.5);
    auto fadein = FadeIn::create(0.5);
    auto sequence = Sequence::create(fadein, fadeout, fadein->clone(), fadeout->clone(),
                                     fadein->clone(), nullptr);
    return (sequence);
}

Action *Utils::get_shield_blink_animation() {
    auto fadeout = FadeOut::create(0.5);
    auto fadein = FadeIn::create(0.5);
    auto callback = CallFuncN::create(
            [&](Node *sender) {
                sender->setVisible(false);

            });
    auto sequence = Sequence::create(fadeout, fadein, fadeout->clone(), fadein->clone(), callback,
                                     nullptr);
    return (sequence);
}

Sprite *Utils::get_player() {
    auto winSize = Director::getInstance()->getVisibleSize();
    int current_shooter = UserLocalStore::get_current_shooter();
    Sprite *player = Sprite::create(shooters[current_shooter]);
    player->
            setPosition(Vec2(static_cast<float>(winSize.width * 0.5),
                             static_cast<float>(winSize.height * 0.25)));
    return (player);
}

Action *Utils::get_bonus_animation(int type) {
    switch (type) {
        case BONUS_SPEED:
            return (get_bonus_speed_anim());
        case BONUS_BULLET:
            return (get_bonus_bullet_anim());
        case BONUS_POWER:
            return (get_bonus_power_anim());
        default:
            break;
    }
    return (nullptr);
}

Action *Utils::get_continue_anim() {
    auto spritecache = SpriteFrameCache::getInstance();
    spritecache->addSpriteFramesWithFile("spritesheet/continue_animation.plist");
    Vector<SpriteFrame *> frames;
    for (int i = 0; i < 33; i++) {
        stringstream ss;
        ss << "continue_" << i << ".png";
        frames.pushBack(spritecache->getSpriteFrameByName(ss.str()));
    }
    Animation *anim = Animation::createWithSpriteFrames(frames, 0.18f);
    CCAction *action = CCRepeatForever::create(CCAnimate::create(anim));
    return (action);
}

Action *Utils::get_bonus_power_anim() {
    auto spritecache = SpriteFrameCache::getInstance();
    spritecache->addSpriteFramesWithFile("spritesheet/bonus_power.plist");
    Vector<SpriteFrame *> frames;
    for (int i = 0; i < 4; i++) {
        stringstream ss;
        ss << "bonus_power_" << i << ".png";
        frames.pushBack(spritecache->getSpriteFrameByName(ss.str()));
    }
    Animation *anim = Animation::createWithSpriteFrames(frames, 0.08f);
    CCAction *action = CCRepeatForever::create(CCAnimate::create(anim));
    return (action);
}

int Utils::get_bullet_nbr(int type) {
    switch (type) {
        case DOUBLE_TANK:
            return (2);
        case SIDEWAY_TANK:
            return (3);
        case TRIPLE_TANK:
            return (3);
        default:
            break;
    }
    return (0);
}

Action *Utils::get_bonus_bullet_anim() {
    auto spritecache = SpriteFrameCache::getInstance();
    spritecache->addSpriteFramesWithFile("spritesheet/bonus_bullet.plist");
    Vector<SpriteFrame *> frames;
    for (int i = 0; i < 4; i++) {
        stringstream ss;
        ss << "bonus_bullet_" << i << ".png";
        frames.pushBack(spritecache->getSpriteFrameByName(ss.str()));
    }
    Animation *anim = Animation::createWithSpriteFrames(frames, 0.08f);
    CCAction *action = CCRepeatForever::create(CCAnimate::create(anim));
    return (action);
}

Action *Utils::get_bonus_speed_anim() {
    auto spritecache = SpriteFrameCache::getInstance();
    spritecache->addSpriteFramesWithFile("spritesheet/bonus_speed.plist");
    Vector<SpriteFrame *> frames;
    for (int i = 0; i < 4; i++) {
        stringstream ss;
        ss << "bonus_speed_" << i << ".png";
        frames.pushBack(spritecache->getSpriteFrameByName(ss.str()));
    }
    Animation *anim = Animation::createWithSpriteFrames(frames, 0.08f);
    CCAction *action = CCRepeatForever::create(CCAnimate::create(anim));
    return (action);
}

int Utils::array_sum(int *array, int size) {
    int sum = 0;

    for (int a = 0; a < size; a++)
        sum += array[a];
    return (sum);
}

bool Utils::is_into_list(int *list, int list_size, int value) {
    for (int i = 0; i < list_size; i++) {
        if (list[i] == value)
            return (true);
    }
    return (false);
}

int *Utils::get_complex_distribution_points(int *distribution, int total_nbr, int line_type,
                                            int sq_nbr) {
    int type = line_type - 6;
    for (int i = 0; i < sq_nbr; i++)
        distribution[i] = static_cast<int>(DISTRIB_COMPLEX[type][i] * total_nbr);
    return (distribution);
}

int *Utils::get_simple_distribution_points(int *distribution, int total_nbr, int line_type,
                                           int sq_nbr) {
    int type = line_type - 4;
    for (int i = 0; i < sq_nbr; i++)
        distribution[i] = static_cast<int>(DISTRIB_SIMPLE[type][i] * total_nbr);
    return (distribution);
}

int *Utils::get_startup_distribution_points(int *distribution, int total_number, int nbr_of_group) {
    int t = 0;
    int loose_pcc = total_number / nbr_of_group;

    for (int i = 1; i < nbr_of_group; i++, t++) {
        int ten_pcc = static_cast<int>(loose_pcc * 0.1);
        int rand_num = cocos2d::RandomHelper::random_int((loose_pcc - ten_pcc),
                                                         (loose_pcc + ten_pcc));
        distribution[t] = rand_num;
    }
    int numbers_total = array_sum(distribution, nbr_of_group - 1);
    distribution[t] = total_number - numbers_total;
    return (distribution);
}

const std::string Utils::get_reduced_value(float value, int type) {
    if (type == VALUE_WITH_POINT) {
        if (value > 1000000000)
            return (StringUtils::format("%.1fB pts", value / 1000000000));
        if (value > 1000000)
            return (StringUtils::format("%.1fM pts", value / 1000000));
        if (value > 1000)
            return (StringUtils::format("%.1fK pts", value / 1000));
        return (StringUtils::format("%1.f pts", value));
    } else if (type == VALUE_SIMPLE) {
        if (value > 1000000000)
            return (StringUtils::format("%.1fB", value / 1000000000));
        if (value > 1000000)
            return (StringUtils::format("%.1fM", value / 1000000));
        if (value > 1000)
            return (StringUtils::format("%.1fK", value / 1000));
        return (StringUtils::format("%1.f", value));
    } else if (type == VALUE_WITH_PLUS) {
        if (value > 1000000000)
            return (StringUtils::format("+%.1fB pts", value / 1000000000));
        if (value > 1000000)
            return (StringUtils::format("+%.1fM pts", value / 1000000));
        if (value > 1000)
            return (StringUtils::format("+%.1fK pts", value / 1000));
        return (StringUtils::format("+%1.f pts", value));
    } else {
        if (value > 1000000000)
            return (StringUtils::format("PERFECT\n+%.1fB pts", value / 1000000000));
        if (value > 1000000)
            return (StringUtils::format("PERFECT\n+%.1fM pts", value / 1000000));
        if (value > 1000)
            return (StringUtils::format("PERFECT\n+%.1fK pts", value / 1000));
        return (StringUtils::format("PERFECT\n+%1.f pts", value));
    }
}

float Utils::get_spawn_y(int current_type, int next_type, float next_line_size[2]) {
    auto screen_height = Director::getInstance()->getVisibleSize().height;
    auto line_height = next_line_size[HEIGHT] / 2;
    if (current_type >= LINE_TYPE_STARTUP_2 && current_type <= LINE_TYPE_STARTUP_5)
        return (-line_height);
    if (current_type == LINE_TYPE_SIMPLE_OF_4 && next_type == LINE_TYPE_SIMPLE_OF_4)
        return static_cast<float>(screen_height * 0.50 + line_height);
    if (current_type == LINE_TYPE_SIMPLE_OF_4 && next_type == LINE_TYPE_SIMPLE_OF_5)
        return static_cast<float>(screen_height * 0.30 + line_height);
    if (current_type == LINE_TYPE_SIMPLE_OF_5 && next_type == LINE_TYPE_SIMPLE_OF_5)
        return static_cast<float>(screen_height * 0.50 + line_height);
    if (current_type > LINE_TYPE_SIMPLE_OF_5 && next_type == LINE_TYPE_SIMPLE_OF_5)
        return static_cast<float>(screen_height * 0.30 + line_height);
    if (current_type == LINE_TYPE_SIMPLE_OF_5 && next_type > LINE_TYPE_SIMPLE_OF_5)
        return static_cast<float>(screen_height * 0.25 + line_height);
    if (current_type > LINE_TYPE_SIMPLE_OF_5 && next_type > LINE_TYPE_SIMPLE_OF_5)
        return static_cast<float>(screen_height * 0.23 + line_height);
    return (-1);
}

int Utils::get_bullet_shoot_index(int tank_type) {
    switch (tank_type) {
        case DOUBLE_TANK:
            return (0);
        case SIDEWAY_TANK:
            return (1);
        case TRIPLE_TANK:
            return (2);
        default:
            return (-1);
    }
}

int Utils::need_background_transition(int step) {
    if (step == 10) {
        return (5);
    } else if (step == 30) {
        return (10);
    }
    return (-1);
}

int Utils::get_shooter_type(int shooter_id) {

    if (shooter_id >= 0 && shooter_id < 6) {
        return (STARTER_TANK);
    } else if (shooter_id >= 6 && shooter_id < 11) {
        return (POWER_TANK);
    } else if (shooter_id >= 11 && shooter_id < 14) {
        return (MOVEMENT_TANK);
    } else if (shooter_id >= 14 && shooter_id < 17) {
        return (DOUBLE_TANK);
    } else if (shooter_id >= 17 && shooter_id < 20) {
        return (SHIELD_TANK);
    } else if (shooter_id >= 20 && shooter_id < 24) {
        return (SPEED_TANK);
    } else if (shooter_id >= 24 && shooter_id < 27) {
        return (SIDEWAY_TANK);
    } else if (shooter_id >= 27 && shooter_id < 31) {
        return (TRIPLE_TANK);
    }
    return (NULL);
}

int Utils::get_random_line_type() {
    int result = get_random_number(0, 30);
    if (result > 7)
        return (LINE_TYPE_SIMPLE_OF_5);
    else
        return (random_type[result]);
}
float Utils::get_random_float_number(float min, float max) {
    assert(max > min);
    float random = ((float) rand()) / (float) RAND_MAX;
    float range = max - min;
    return (random*range) + min;
}

int Utils::get_random_number(int min, int max) {
    return (cocos2d::RandomHelper::random_int(min, max));
}

int *Utils::get_container_range_research(int type) {
    int *range = new int[2];

    switch (type) {
        case LINE_TYPE_SIMPLE_OF_4:
            range[0] = 4;
            range[1] = 6;
            break;
        case LINE_TYPE_SIMPLE_OF_5:
            range[0] = 7;
            range[1] = 9;
            break;
        case LINE_TYPE_COMPLEX_0:
            range[0] = 10;
            range[1] = 11;
            break;
        case LINE_TYPE_COMPLEX_1:
            range[0] = 12;
            range[1] = 13;
            break;
        case LINE_TYPE_COMPLEX_2:
            range[0] = 14;
            range[1] = 15;
            break;
        case LINE_TYPE_COMPLEX_3:
            range[0] = 16;
            range[1] = 17;
            break;
        case LINE_TYPE_COMPLEX_4:
            range[0] = 18;
            range[1] = 19;
            break;
        case LINE_TYPE_COMPLEX_5:
            range[0] = 20;
            range[1] = 21;
            break;
        case LINE_TYPE_COMPLEX_6:
            range[0] = 22;
            range[1] = 23;
            break;
        case LINE_TYPE_COMPLEX_7:
            range[0] = 24;
            range[1] = 25;
            break;
        default:
            break;
    }
    return (range);
}

Vec2 Utils::get_coordinate_from_id(int id, int column) {
    return (Vec2(id % column, id / column));
}

float Utils::get_finger_move_factor(float x, int shooter_type) {
    auto winSize = Director::getInstance()->getVisibleSize();
    float old_min = 0;
    float old_max = winSize.width;
    float new_min = -winSize.width / 2;
    float new_max = winSize.width / 2;
    if (shooter_type == MOVEMENT_TANK) {
        new_min = static_cast<float>(-winSize.width / 1.5);
        new_max = static_cast<float>(winSize.width / 1.5);
    }
    return ((((x - old_min) * (new_max - new_min)) / (old_max - old_min)) + new_min);
}
