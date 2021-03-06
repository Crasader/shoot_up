/*
##
## STAINDIO PROJECT, 2018
## shootup
## File description:
## line object class, lines contains Square Object
*/

#include    "Line.h"
#include    "Square.h"
#include    "Utils.h"
#include    "ShootUp.h"
#include    "GameScene.h"
#include    "UserLocalStore.h"

Line::Line(int type) {
}

Line::~Line() {
}

int Line::get_random_index(int *choosen, int max) {
    while (1) {
        int rand = Utils::get_random_number(0, max);
        if (!Utils::is_into_list(choosen, max, rand))
            return (rand);
    }
}

void Line::attach_star_bonus() {
    int selected = Utils::get_random_number(0, 4);
    auto child = getChildByTag(selected);
    if (!child)
        return;
    Square *sq = ((Square *) child);
    sq->star_bonus = 1;
    Sprite *star = Sprite::create(STAR_BONUS_TEXTURE);
    star->setScale(0.85f);
    star->setContentSize(Size(sq->getContentSize().width / 2, sq->getContentSize().height / 2));
    sq->addChild(star, 200);
    star->setTag(STAR_BONUS_TAG);
    star->setPosition(Vec2(sq->getContentSize().width / 2,
                           star->getPositionY() + star->getContentSize().height / 2));
}

void Line::attach_chest_bonus() {
    int selected_square = Utils::get_random_number(0, 4);
    auto square_child = getChildByTag(selected_square);
    auto batchnode = getChildByTag(LINE_BATCH_TAG);
    if (!batchnode)
        return;
    Square *sq = ((Square*) square_child);
    sq->chest_bonus = true;
    auto square_sprite = batchnode->getChildByTag(selected_square);
    if (!square_sprite)
        return;
    ((Sprite *) square_sprite)->setSpriteFrame(DEFAULT_CHEST_TEXTURE);
}

void Line::change_square_color(int square_id, int color) {
    if (((Square *) getChildByTag(square_id))->chest_bonus)
        return;
    ((Square *) getChildByTag(square_id))->initial_color = color;
    auto batchnode = getChildByTag(LINE_BATCH_TAG);
    if (!batchnode)
        return;
    auto sprite_child = batchnode->getChildByTag(square_id);
    SpriteFrame *tmp = (SpriteFrame *) basic_colors[color];
    if (((Sprite *) sprite_child)->getSpriteFrame() != tmp)
        ((Sprite *) sprite_child)->setSpriteFrame(basic_colors[color]);
}

void Line::assign_color(int sq_id, int m_factor, int current_point) {
    float point = static_cast<float>(current_point);
    if (point <= static_cast<float>(m_factor * 0.85))
        change_square_color(sq_id, GREEN);
    else if (point <= static_cast<float>(m_factor * 0.95))
        change_square_color(sq_id, YELLOW);
    else if (point <= static_cast<float>(m_factor * 1.05))
        change_square_color(sq_id, ORANGE);
    else
        change_square_color(sq_id, RED);
}

int Line::get_special_shooter_total_point(int total) { // AUGMENTATION DU NIVEAU QUAND SPECIAL SHOOTER
    int shooter_type = Utils::get_shooter_type(UserLocalStore::get_current_shooter());
    switch (shooter_type) {
        case SPEED_TANK:
            return static_cast<int>(total * 1.5);
        case POWER_TANK:
            return static_cast<int>(total * 1.5);
        case TRIPLE_TANK:
            return (total * 2);
        case DOUBLE_TANK:
            return static_cast<int>(total * 1.5);
        default:
            return 0;
    }
}

void Line::assign_line_points(int h_factor, int line_generated) { // POUR LES LIGNES DE 4 ou 5
    int total = 0;
    int *distrib = new int[this->square_nbr];
    int *choosen = new int[this->square_nbr];

    if (get_line_type() == LINE_TYPE_SIMPLE_OF_4) {
        total = h_factor + ((line_generated / 10) * h_factor);
    } else {
        int min_h = static_cast<int>(h_factor +
                                     ceil(static_cast<float>(h_factor * 0.6)));
        int max_h = static_cast<int>(h_factor +
                                     ceil(static_cast<float>(h_factor * 0.8)));
        total = Utils::get_random_number(min_h, max_h) +
                (Utils::get_random_number(0, line_generated) / INCREASE_POINTS_FACTOR) * total;
    }
    total += get_special_shooter_total_point(total);
    half_total = total / 2;
    distrib = Utils::get_simple_distribution_points(distrib, total, get_line_type(), this->square_nbr);
    int index = 0;
    while (1) {
        auto child = getChildByTag(index);
        Square *sq = ((Square *) child);
        if (!child || !sq)
            return;
        choosen[index] = get_random_index(choosen, this->square_nbr - 1);
        sq->assign_point(distrib[choosen[index]]);
        assign_color(index, total / this->square_nbr, distrib[choosen[index]]);
        index++;
    }
}

void Line::assign_complex_line_points(int h_factor,
                                      int line_generated) { // POINTS FOR COMPLEX STRUCT (MARTIN'S SYSTEM)
    int type = get_line_type();
    int *distrib = new int[this->square_nbr];
    int index = 0;
    int min_h = static_cast<int>(h_factor +
                                 ceil(static_cast<float>(h_factor * 0.3)));
    int max_h = static_cast<int>(h_factor +
                                 ceil(static_cast<float>(h_factor * 0.4)));
    int total = static_cast<int>(Utils::get_random_number(min_h, max_h) * 1.4);
    total += get_special_shooter_total_point(total) +
             (Utils::get_random_number(0, line_generated) / INCREASE_POINTS_FACTOR) * total;
    half_total = total / 2;
    distrib = Utils::get_complex_distribution_points(distrib, total, type, this->square_nbr);
    while (1) {
        auto square_child = getChildByTag(index);
        if (!square_child)
            break;
        ((Square *) square_child)->assign_point(distrib[index]);
        assign_color(index, total / this->square_nbr, distrib[index]);
        index++;
    }
}

void
Line::assign_startup_line_points(int h_factor) {  // POINTS FOR STARTUP STRUCT (MARTIN'S SYSTEM)
    int type = get_line_type();
    type -= 1;
    int lines_nbr = square_nbr / 5;
    h_factor = H_FACTORE_STARTUP_STRUCT(h_factor);
    for (int i = 0; i < lines_nbr; i++) {
        float ratio = LINE_DIVISION[type][i];
        int *distrib = new int[5];
        distrib = Utils::get_startup_distribution_points(distrib,
                                                         static_cast<int>(h_factor * ratio),
                                                         5);
        int index = i * 5;
        int k = 0;
        auto batchnode = getChildByTag(LINE_BATCH_TAG);
        for (int j = index; j < (i * 5 + 5); j++, k++) {
            auto child = getChildByTag(j);
            if (!child)
                break;
            Square *sq = ((Square *) child);
            sq->initial_color = GREEN;
            if (batchnode) {
                auto sprite = batchnode->getChildByTag(sq->getTag());
                Sprite *e = ((Sprite *) sprite);
                char color[15];
                sprintf(color, "%s", "red.png");
                e->setSpriteFrame(color);
            }
            sq->assign_point(distrib[k]);
        }
    }
}

Size Line::get_line_size(int type) {
    auto winSize = Director::getInstance()->getVisibleSize();
    if (type == LINE_TYPE_SIMPLE_OF_4)
        return (Size(winSize.width,
                     static_cast<float>(winSize.height / SQUARE_SIZE_4_HEIGHT)));
    else if (type == LINE_TYPE_SIMPLE_OF_5)
        return (Size(winSize.width,
                     static_cast<float>(winSize.height / SQUARE_SIZE_5_HEIGHT)));
    float line_nbr = 0;
    if (type == LINE_TYPE_STARTUP_3)
        line_nbr = 3;
    else if (type == LINE_TYPE_STARTUP_4)
        line_nbr = 4;
    else if (type == LINE_TYPE_STARTUP_5)
        line_nbr = 5;
    else
        line_nbr = 3;
    return (Size(winSize.width,
                 (static_cast<float>(winSize.height / SQUARE_SIZE_5_HEIGHT)) * line_nbr));
}

/********************************** BLOCKS TRANSLATIONS ****************************/
void
Line::apply_animation(Line *l, Sprite *s, Square *square, int struct_nbr, int index) {
    if (struct_nbr == 3) {
        if (index == 3)
            apply_middle_left_translation(square, s, square->get_rect_size(),
                                          l->line_size);
        if (index == 4)
            apply_middle_right_translation(square, s, square->get_rect_size(),
                                           l->line_size);
        if (index == 5)
            apply_full_translation(square, s, square->get_rect_size(), l->line_size);
    } else if (struct_nbr == 4) {
        if (index == 0)
            apply_double_left_translation(square, s, square->get_rect_size());
        if (index == 2)
            apply_double_right_translation(square, s, square->get_rect_size());
        if (index == 5)
            apply_double_left_translation(square, s, square->get_rect_size());
        if (index == 7)
            apply_double_right_translation(square, s, square->get_rect_size());
    }
}

void Line::apply_full_translation(Square *s, Sprite *sprite, float rect_size[2],
                                  float line_size[2]) {
    float block_width = rect_size[WIDTH];

    auto actionMove1 = MoveBy::create(0.9,
                                      Vec2((line_size[WIDTH] - block_width / 2) -
                                           s->getPosition().x, 0));
    auto actionMove2 = MoveBy::create(0.9,
                                                              Vec2(-((line_size[WIDTH] - block_width / 2) -
                                                                     s->getPosition().x),
                                                                   0));
    auto seq = Sequence::create(actionMove1, actionMove2, nullptr);
    auto seq2 = Sequence::create(actionMove1->clone(), actionMove2->clone(), nullptr);
    s->runAction(RepeatForever::create(seq));
    sprite->runAction(RepeatForever::create(seq2));
}

void Line::apply_middle_left_translation(Square *s, Sprite *sprite, float rect_size[2],
                                         float line_size[2]) {
    float block_width = rect_size[WIDTH];
    auto winSize = Director::getInstance()->getVisibleSize();
    int distance = static_cast<int>(((line_size[WIDTH] / 2) - block_width));
    auto actionMove1 = MoveBy::create(0.8, Vec2(distance,
                                                0));
    auto actionMove2 = MoveBy::create(0.8, Vec2(-distance, 0));
    auto seq = Sequence::create(actionMove1, actionMove2, nullptr);
    auto seq2 = Sequence::create(actionMove1->clone(), actionMove2->clone(), nullptr);
    s->runAction(RepeatForever::create(seq));
    sprite->runAction(RepeatForever::create(seq2));
}

void Line::apply_middle_right_translation(Square *s, Sprite *sprite, float rect_size[2],
                                          float line_size[2]) {
    float block_width = rect_size[0];
    auto winSize = Director::getInstance()->getVisibleSize();
    int distance = static_cast<int>((line_size[WIDTH] - block_width / 2) -
                                    (line_size[WIDTH] / 2 + block_width / 2));
    auto actionMove1 = MoveBy::create(0.8, Vec2(-distance, 0));
    auto actionMove2 = MoveBy::create(0.8, Vec2(distance, 0));
    auto seq = Sequence::create(actionMove1, actionMove2, nullptr);
    auto seq2 = Sequence::create(actionMove1->clone(), actionMove2->clone(), nullptr);
    s->runAction(RepeatForever::create(seq));
    sprite->runAction(RepeatForever::create(seq2));
}

void
Line::apply_double_left_translation(Square *s, Sprite *sprite, float rect_size[2]) {
    float block_width = rect_size[0];
    auto actionMove1 = MoveBy::create(0.8, Vec2(block_width, 0));
    auto actionMove2 = MoveBy::create(0.8, Vec2(-block_width, 0));
    auto seq = Sequence::create(actionMove1, actionMove2, nullptr);
    auto seq2 = Sequence::create(actionMove1->clone(), actionMove2->clone(), nullptr);
    s->runAction(RepeatForever::create(seq));
    sprite->runAction(RepeatForever::create(seq2));
}

void
Line::apply_double_right_translation(Square *s, Sprite *sprite, float rect_size[2]) {
    float block_width = rect_size[0];
    auto winSize = Director::getInstance()->getVisibleSize();
    auto actionMove1 = MoveBy::create(0.8, Vec2(-block_width, 0));
    auto actionMove2 = MoveBy::create(0.8, Vec2(block_width, 0));
    auto seq = Sequence::create(actionMove1, actionMove2, nullptr);
    auto seq2 = Sequence::create(actionMove1->clone(), actionMove2->clone(), nullptr);
    s->runAction(RepeatForever::create(seq));
    sprite->runAction(RepeatForever::create(seq2));
}
/********************************** BLOCKS TRANSLATIONS ****************************/


/********************************** LINE STRUCTURES *****************************/
Vec2 Line::get_square_pos_into_line(int requiered_pos, float rect_size[2], int line_size) {
    Vec2 req_pos;
    if (line_size == SQUARE_SIZE_LINE_OF_5)
        req_pos = Utils::get_coordinate_from_id(requiered_pos, SQUARE_SIZE_LINE_OF_5);
    else
        req_pos = Utils::get_coordinate_from_id(requiered_pos, SQUARE_SIZE_LINE_OF_4);
    return (Vec2((rect_size[WIDTH] / 2) + (req_pos.x * rect_size[WIDTH]),
                 ((rect_size[HEIGHT]) / 2) + (req_pos.y * (rect_size[HEIGHT]))));
}

/********************************** LINE STRUCTURES *****************************/


/********************************** BATCHNODE LOADING *****************************/

SpriteBatchNode *Line::get_batch() {
    SpriteBatchNode *spriteBatchNode;
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile(BLOCK_COLOR_PLIST);
    spriteBatchNode = SpriteBatchNode::create(SPRITESHEET_BLOCK_COLORS);
    spriteBatchNode->setAnchorPoint(Vec2(0, 0));
    spriteBatchNode->setTag(LINE_BATCH_TAG);
    return (spriteBatchNode);
}

Sprite *Line::get_texture(int i, float *sq_size) {
    auto sprite = Sprite::createWithSpriteFrameName(DEFAULT_SQUARE_TEXTURE);
    sprite->setAnchorPoint(Vec2(0.5, 0.5));
    sprite->setTag(i);
    sprite->setScaleY(
            static_cast<float>((sq_size[HEIGHT] * 0.8) / sprite->getContentSize().height));
    sprite->setScaleX(
            static_cast<float>(sq_size[WIDTH] * 0.8) / sprite->getContentSize().width);
    return (sprite);
}

/********************************** BATCHNODE LOADING *****************************/


/********************************** LINES LOADING *****************************/

void Line::load_startup_struct(Line *l) {
    SpriteBatchNode *spriteBatchNode = get_batch();
    spriteBatchNode->setContentSize(l->getContentSize());
    l->addChild(spriteBatchNode);
    int total_square = l->square_nbr;
    for (int i = 0; i < total_square; i++) {
        Square *sq = Square::create(SQUARE_SIZE_LINE_OF_5);
        if (sq) {
            sq->setTag(i);
            sq->setPosition(get_square_pos_into_line(i, sq->get_rect_size(), SQUARE_SIZE_LINE_OF_5));
            l->addChild(sq);
            auto sprite = get_texture(i, sq->get_rect_size());
            sprite->setAnchorPoint(Vec2(0.5, 0.5));
            sprite->setPosition(Vec2(sq->getPositionX(),
                                     static_cast<float>((sq->getPositionY() - sq->getContentSize().height / 2) +
                                                        (0.55 * sq->getContentSize().height))));
            spriteBatchNode->addChild(sprite);
        }
    }
}

void Line::load_complex_struct(Line *l, int struct_number) {
    SpriteBatchNode *spriteBatchNode = get_batch();
    spriteBatchNode->setContentSize(l->getContentSize());
    l->addChild(spriteBatchNode);

    int size = get_square_nbr_per_line(struct_number);
    struct_number -= 6;
    const int *pos_struct = SQUARE_POSITION_STRUCTURE[struct_number];

    for (int i = 0; i < size; i++) {
        Square *sq = Square::create(SQUARE_SIZE_LINE_OF_5);
        if (sq) {
            sq->setTag(i);
            sq->setPosition(
                    get_square_pos_into_line(pos_struct[i], sq->get_rect_size(),
                                        SQUARE_SIZE_LINE_OF_5));
            l->addChild(sq);
            auto sprite = get_texture(i, sq->get_rect_size());
            sprite->setAnchorPoint(Vec2(0.5, 0.5));
            sprite->setPosition(Vec2(sq->getPositionX(),
                                     static_cast<float>((sq->getPositionY() - sq->getContentSize().height / 2) +
                                                        (0.55 * sq->getContentSize().height))));
            spriteBatchNode->addChild(sprite);
            if (struct_number == 3 || struct_number == 4)
                apply_animation(l, sprite, sq, struct_number, i);
        }
    }
}

void Line::load_simple_line_5(Line *l) {
    SpriteBatchNode *spriteBatchNode = get_batch();
    spriteBatchNode->setContentSize(l->getContentSize());
    l->addChild(spriteBatchNode);

    for (int i = 0; i < 5; i++) {
        Square *sq = Square::create(SQUARE_SIZE_LINE_OF_5);
        if (sq) {
            sq->setTag(i);
            sq->setPosition(
                    get_square_pos_into_line(i, sq->get_rect_size(), SQUARE_SIZE_LINE_OF_5));
            l->addChild(sq);
            auto sprite = get_texture(i, sq->get_rect_size());
            sprite->setAnchorPoint(Vec2(0.5, 0.5));
            sprite->setPosition(Vec2(sq->getPositionX(),
                                     static_cast<float>((sq->getPositionY() - sq->getContentSize().height / 2) +
                                                        (0.55 * sq->getContentSize().height))));
            spriteBatchNode->addChild(sprite);
        }
    }
}

void Line::load_simple_line_4(Line *l) {
    SpriteBatchNode *spriteBatchNode = get_batch();
    spriteBatchNode->setContentSize(l->getContentSize());
    l->addChild(spriteBatchNode);

    for (int i = 0; i < 4; i++) {
        Square *sq = Square::create(SQUARE_SIZE_LINE_OF_4);
        if (sq) {
            sq->setPosition(
                    get_square_pos_into_line(i, sq->get_rect_size(), SQUARE_SIZE_LINE_OF_4));
            sq->setTag(i);
            l->addChild(sq);
            auto sprite = get_texture(i, sq->get_rect_size());
            sprite->setAnchorPoint(Vec2(0.5, 0.5));
            sprite->setPosition(Vec2(sq->getPositionX(),
                                     static_cast<float>((sq->getPositionY() - sq->getContentSize().height / 2) +
                                                        (0.55 * sq->getContentSize().height))));
            spriteBatchNode->addChild(sprite);

        }
    }
}

int Line::get_line_type() {
    return (line_type);
}

int Line::get_square_nbr_per_line(int type) {
    return (SQUARE_NBR_PER_LINE[type - 6]);
}

void Line::load_squares_into_line(Line *l, int type) {
    if (type > LINE_TYPE_SIMPLE_OF_5) {
        l->line_type = type;
        l->square_nbr = get_square_nbr_per_line(type);
        load_complex_struct(l, type);
        return;
    }
    switch (type) {
        case LINE_TYPE_SIMPLE_OF_4:
            l->square_nbr = 4;
            l->line_type = LINE_TYPE_SIMPLE_OF_4;
            load_simple_line_4(l);
            break;
        case LINE_TYPE_SIMPLE_OF_5:
            l->square_nbr = 5;
            l->line_type = LINE_TYPE_SIMPLE_OF_5;
            load_simple_line_5(l);
            break;
        case LINE_TYPE_STARTUP_2:
            l->square_nbr = 10;
            l->line_type = LINE_TYPE_STARTUP_2;
            load_startup_struct(l);
            break;
        case LINE_TYPE_STARTUP_3:
            l->square_nbr = 15;
            l->line_type = LINE_TYPE_STARTUP_3;
            load_startup_struct(l);
            break;
        case LINE_TYPE_STARTUP_4:
            l->square_nbr = 20;
            l->line_type = LINE_TYPE_STARTUP_4;
            load_startup_struct(l);
            break;
        case LINE_TYPE_STARTUP_5:
            l->square_nbr = 25;
            l->line_type = LINE_TYPE_STARTUP_5;
            load_startup_struct(l);
            break;
        default:
            break;
    }
}

/********************************** LINES LOADING *****************************/

/********************************** BASICS *****************************/

void Line::set_active(int factor_h, int line_generated) {
    setVisible(true);
    line_active = true;
    if (get_line_type() > LINE_TYPE_SIMPLE_OF_5)
        assign_complex_line_points(factor_h, line_generated);
    else if (get_line_type() >= LINE_TYPE_SIMPLE_OF_4)
        assign_line_points(factor_h, line_generated);
    else
        assign_startup_line_points(factor_h);
}

void Line::check_for_transition_line() {
    if (this->getScale() == 0.85f) {
        this->setScale(1);
        this->setAnchorPoint(Vec2(0, 0));
    }
}

void Line::reset() {
    int index = 0;
    auto batchnode = getChildByTag(LINE_BATCH_TAG);
    check_for_transition_line();
    while (1) {
        auto child = this->getChildByTag(index);
        if (!child)
            break;
        Square *sq = ((Square *) child);
        sq->reset_square();
        auto sprite = batchnode->getChildByTag(sq->getTag());
        if (!sprite->isVisible())
            sprite->setVisible(true);
        if (!sq->isVisible())
            sq->setVisible(true);
        index++;
    }
    line_animated = false;
    half_total = 0;
    line_active = false;
    setVisible(false);
    auto winSize = Director::getInstance()->getVisibleSize();
    setPosition(initial_pos);
}

Line *Line::create(int type) {
    auto winSize = Director::getInstance()->getVisibleSize();
    Line *l = new Line(type);
    l->setAnchorPoint(Vec2::ZERO);
    auto size = get_line_size(type);
    l->line_size[WIDTH] = size.width;
    l->line_size[HEIGHT] = size.height;
    l->half_total = 0;
    l->line_animated = false;
    l->initial_pos = Vec2(0, winSize.height + l->line_size[HEIGHT]);
    l->line_active = false;
    l->setContentSize(Size(l->line_size[WIDTH], l->line_size[HEIGHT]));
    l->setPosition(l->initial_pos);
    load_squares_into_line(l, type);
    l->setVisible(false);
    return (l);
    CC_SAFE_DELETE(l);
}


