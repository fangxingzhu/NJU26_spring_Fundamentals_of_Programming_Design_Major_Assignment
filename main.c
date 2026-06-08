//#define TEST_MODE //测试模式，测试完注释掉即可
#include "sts_io.h"
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define MAX_PLAYER_HP 80 //玩家最大生命值
#define MAX_PLAYER_EP 3  //玩家最大能量值
#define MAX_ENEMY_HP 40  //敌人最大生命值
#define CARD_TYPES 7 //可使用卡牌的种类数
struct PLAYER {
    int HP;
    int DEFENCE;
    int EP;
    bool Vulnerable;//受到的攻击伤害增加50%。（1，涉及分数部分都向下取整）（2，格挡部分也计算在内，相当于敌人打出伤害增加50%）
    //(3,不包括放血）(4.单回合效果）
    bool Weak;//造成的攻击伤害减少25%。（1，涉及分数部分都向下取整）（2，不包括放血）（3，单回合效果）
};//玩家属性结构体
typedef struct PLAYER PLAYER;
struct ENEMY {
    int HP;
    int DEFENCE;
    bool Vulnerable;
    bool Weak;
    int Poison;//每回合失去等同于层数的生命值，随后层数减1。（1，不考虑格挡）（2，持续多回合效果）
};//敌人属性结构体
typedef struct ENEMY ENEMY;
PLAYER player={MAX_PLAYER_HP,0,MAX_PLAYER_EP,false,false};
ENEMY enemy={MAX_ENEMY_HP,0,false,false,0};

struct Card
{
    char name[100];
    int cost;
    char description[200];
    int type;   // 卡牌种类：0=打击，1=防御，2=放血，3=暴走
    int damage; //
};//卡牌结构体
typedef struct Card Card;
Card cards[CARD_TYPES]={{"打击",1,"对敌人造成 6 点伤害",0,6},
{"防御",1,"增加 5 点格挡",1,0},
{"放血",0,"对自己造成 3 点伤害，恢复 1 点能量",2,0},
{"暴走",1,"对敌人造成 8 点伤害。每次打出这张牌时，这张牌在本局战斗中的伤害增加 5 点。",3,8},
{"鲁莽",1,"对敌人造成 12 点伤害，同时自己本回合获得虚弱状态",4,12},
{"中毒",1,"使敌人获得中毒状态5层，而自己因为吸入毒气本回合获得易伤状态",5,0},
{"传染",0,"使敌人本回合获得和自己一样的负面状态",6,0}
};//每种卡牌的基本信息

//以下是卡牌堆
Card all_cards_pile[11]={
    {"打击",1,"对敌人造成 6 点伤害",0,6},
{"打击",1,"对敌人造成 6 点伤害",0,6},
{"打击",1,"对敌人造成 6 点伤害",0,6},
{"防御",1,"增加 5 点格挡",1,0},
{"防御",1,"增加 5 点格挡",1,0},
{"防御",1,"增加 5 点格挡",1,0},
{"放血",0,"对自己造成 3 点伤害，恢复 1 点能量",2,0},
{"暴走",1,"对敌人造成 8 点伤害。每次打出这张牌时，这张牌在本局战斗中的伤害增加 5 点。",3,8},
{"鲁莽",1,"对敌人造成 12 点伤害，同时自己本回合获得虚弱状态",4,12},
{"中毒",1,"使敌人获得中毒状态5层(不叠加），而自己因为吸入毒气本回合获得易伤状态",5,0},
{"传染",0,"使敌人本回合获得和自己一样的状态",6,0}
}; // 总牌池（库）
Card piles[3][11];//piles[0]是抽牌堆，piles[1]是手牌，piles[2]是弃牌堆
int piles_count[3];//各牌堆牌数

void show_main_menu(void);//显示主菜单
void draw_hud(void);//显示玩家和敌人状态栏
void deal_damage_to_player(int damage);//玩家受到伤害
void deal_damage_to_enemy(int damage);//敌人受到伤害
bool check_ep(const Card* card);//检查玩家能量值是否足以打出这张卡牌
void check_player(void);//检查玩家的生命值、能量值上限以及敌人生命值上限（其他已经通过别的函数保障）
bool play_round(int operation);//执行一回合并且检查是否结束一回合游戏
bool judge_of_victory(void);//判断胜负
void reset_game(void);//重置游戏数据
void reset_a_round(void);//重置一回合（归零玩家格挡和抽5张手牌）
void show_cards(int pile_type);//显示卡堆里面的卡牌；
void add_card_to_pile(Card card_type, int pile_type);//添加一张某种类卡牌到指定的牌堆
void draw_one_card(void);//从抽牌堆顶部（最后）抽一张卡牌到手（牌堆）上,并且把这张卡牌从抽牌堆中移去
void reset_hand(int operation);//将打出的手牌删去且把这张手牌后面的牌前移
void init_rand(void);//检查是否已经随机化；否则进行随机化
int randint(int min, int max);//进行范围内的随机化
void shuffle_pile(void);//洗牌抽牌堆
void enemy_turn(void);//敌人回合的行动
int round_count=1;//回合数计数
int enemy_state=1;//敌人状态记录
int transition_round=0;//敌人进入第二状态的回合
void show_enemy_intent(void);//展示敌人意图
int calc_damage(int base, bool attacker_is_player);//用于实现虚弱和易伤逻辑；

//以下是具体的每种卡牌的效果函数
void card_0(Card* card);
void card_1(Card* card);
void card_2(Card* card);
void card_3(Card* card);
void card_4(Card* card);
void card_5(Card* card);
void card_6(Card* card);
typedef void (*PLAY_CARD)(Card* card);
PLAY_CARD player_play_card[CARD_TYPES]={ card_0, card_1, card_2, card_3, card_4, card_5, card_6};
void play_card(Card* card);

void test_logic(void);//自动化检查功能（AI生成）


int main(void) {
    test_logic();
    while (true) {
        show_main_menu();
        if (sts_read_int_range("请输入相应数字以继续: ", 1, 2) == 1){
            sts_puts("游戏正在进行中...");
            reset_game();
            sts_pause(NULL);
            while (true)
            {
                reset_a_round();
                while (true)
                {
                    sts_clear_screen();
                    draw_hud();
                    sts_puts("0.结束回合");
                    show_cards(1);
                    int operation=sts_read_int_range("请输入你要进行的操作: ",0,piles_count[1]);
                    if (play_round(operation))
                        break;
                }
                if (judge_of_victory())
                    break;
            }
        }
        else {
            sts_puts("再见！");
            break;
        }
    } //主循环
    return 0;
}

void show_main_menu(void) {
    sts_clear_screen();
    sts_title("某不知名神人卡牌游戏");
    sts_puts("1. 进入游戏");
    sts_puts("2. 退出游戏");
}

void draw_hud(void){
    sts_print("玩家当前生命值：",player.HP,"/",MAX_PLAYER_HP,"\n");
    sts_print("玩家当前格挡值：",player.DEFENCE,"\n");
    sts_print("玩家当前能量值：",player.EP,"/",MAX_PLAYER_EP,"\n");
    if (player.Vulnerable) sts_print("玩家状态：[易伤]","\n");
    if (player.Weak)       sts_print("玩家状态：[虚弱]","\n");
    sts_print("敌人当前生命值：",enemy.HP,"/",MAX_ENEMY_HP,"\n");
    sts_print("敌人当前格挡值：", enemy.DEFENCE, "\n");
    if (enemy.Vulnerable) sts_print("敌人状态：[易伤]","\n");
    if (enemy.Weak)       sts_print("敌人状态：[虚弱]","\n");
    if (enemy.Poison > 0) sts_print("敌人状态：[中毒]，持续",enemy.Poison,"回合","\n");
    show_enemy_intent();
    sts_puts(" "); //不加这个全部挤在一起太丑了
}

void deal_damage_to_player(int damage){
    if (damage <= player.DEFENCE)
        player.DEFENCE -= damage;
    else
    {
        player.HP -= (damage-player.DEFENCE);
        player.DEFENCE = 0;
    }
    if (player.HP <= 0)
        player.HP = 0;
}

void deal_damage_to_enemy(int damage){
    if (damage <= enemy.DEFENCE)
        enemy.DEFENCE -= damage;
    else
    {
        enemy.HP -= (damage-enemy.DEFENCE);
        enemy.DEFENCE = 0;
    }
    if (enemy.HP <= 0)
        enemy.HP = 0;
}

bool check_ep(const Card* card)
{
   return  player.EP >= card->cost;
}

void check_player(void)
{
    if (player.HP > MAX_PLAYER_HP)
        player.HP = MAX_PLAYER_HP;
    if (player.EP > MAX_PLAYER_EP)
        player.EP = MAX_PLAYER_EP;
    if (enemy.HP > MAX_ENEMY_HP)
        enemy.HP=MAX_ENEMY_HP;
}

bool play_round (int operation)
{
    if (!operation)
    {
        for (int i=0;i<piles_count[1];++i)
            add_card_to_pile(piles[1][i],2);
        piles_count[1]=0;
        enemy_turn();
        return true;
    }
    else
    {
        Card* current_card=&piles[1][operation - 1];
        if (check_ep(current_card))
        {
            if (current_card->type!=3)
                sts_print("玩家打出了", current_card->name, ",", current_card->description, "\n");
            else
                sts_print("玩家打出了暴走，造成 ",current_card->damage," 点伤害" "\n");
            sts_puts("");
            play_card(current_card);
            add_card_to_pile(*current_card, 2);
            reset_hand(operation);
        }
        else
        {
            sts_print("玩家能量值不足！本次操作无效！\n");
            sts_pause(NULL);
        }
        if (player.HP == 0 || enemy.HP == 0)
            return true;
        if (enemy_state == 1 && enemy.HP <= 20) {
            enemy_state = 2;
            transition_round = round_count;
            sts_clear_screen();
            sts_print("敌人进入二阶段！下回合将造成 16 点伤害\n");
            draw_hud();
            sts_pause(NULL);
        }
    }
    return false;
}


bool judge_of_victory(void){
    if (enemy.HP == 0)
    {
        sts_clear_screen();
        sts_puts("敌人死亡！恭喜玩家胜利");
        sts_pause(NULL);//不加这个结算信息一闪而过太快了
        return true;
    }
    if (player.HP == 0)
    {
        sts_clear_screen();
        sts_puts("玩家死亡！玩家失败～～～");
        sts_pause(NULL);
        return true;
    }
    return false;
}

void reset_game(void){
    player.HP = MAX_PLAYER_HP;
    player.EP = MAX_PLAYER_EP;
    player.DEFENCE = 0;
    player.Vulnerable = false;
    player.Weak=false;
    enemy.HP = MAX_ENEMY_HP;
    enemy.DEFENCE = 0;
    enemy.Vulnerable = false;
    enemy.Weak=false;
    enemy.Poison=0;
    piles_count[0] = piles_count[1] = piles_count[2] = 0;
    for (int i=0;i<11;++i)
    {
        add_card_to_pile(all_cards_pile[i], 0);
    }
    shuffle_pile();
    round_count = 1;
    transition_round=0;
    enemy_state=1;
}

void reset_a_round(void)
{
    for (int i=0;i<5;++i)
        draw_one_card();
    player.EP=MAX_PLAYER_EP;
    player.DEFENCE=0;
    player.Vulnerable=false;
    player.Weak=false;
    enemy.Vulnerable=false;
    enemy.Weak=false;
}

void show_cards(int pile_type){
    for (int i=1;i<=piles_count[pile_type];++i)
    {
        if (piles[pile_type][i-1].type!=3)
            sts_print(i,".[",piles[pile_type][i-1].name,"]"," (消耗",piles[pile_type][i-1].cost,"点能量值)"," [功能:",piles[pile_type][i-1].description,"]\n");
        else
        {
            sts_print(i,".[",piles[pile_type][i-1].name,"]"," (消耗",piles[pile_type][i-1].cost,"点能量值)"," [功能:","造成 ",piles[pile_type][i-1].damage);
            sts_print(" 点伤害。每次打出这张牌时，这张牌在本局战斗中的伤害增加 5 点。","]\n");
        }
    }

}

void play_card(Card* card)
{
        player.EP -= card->cost;
        player_play_card[card->type](card);
        check_player();
}

void add_card_to_pile(Card card, int pile_type)
{
    piles[pile_type][piles_count[pile_type]]=card;
    piles_count[pile_type]++;
}

void draw_one_card(void)
{
    if (piles_count[1]>=5)
        return;
    if (piles_count[2]<0)
        return;
    if (piles_count[0]<=0)
    {
        for (int i=0;i<piles_count[2];++i)
            piles[0][i]=piles[2][i];
        piles_count[0]=piles_count[2];
        piles_count[2]=0;
        shuffle_pile();
    }
    Card drawn_card=piles[0][piles_count[0]-1];
    piles_count[0]--;
    add_card_to_pile(drawn_card,1);
}

void reset_hand(int operation)
{
    for (int i=operation;i<piles_count[1];++i)
        piles[1][i-1]=piles[1][i];
    piles_count[1]--;
}

void init_rand(void) {
    static int initialized = 0;
    if (!initialized) {
#ifdef TEST_MODE
        srand(42);                  // 固定种子，测试专用
#else
        srand((unsigned)time(NULL)); // 正常游戏的随机种子
#endif
        initialized = 1;
    }
}

int randint(int min, int max)
{
    init_rand();
    return rand()%(max-min+1)+min;
}

void shuffle_pile(void)
{
    for (int i=piles_count[0]-1;i>0;--i)
    {
        int j=randint(0,i);
        Card temp=piles[0][i];
        piles[0][i]=piles[0][j];
        piles[0][j]=temp;
    }
}

void enemy_turn(void)
{
    if (enemy_state==1)
    {
        if (round_count%3==1)
            enemy.DEFENCE+=6;
        else if (round_count%3==2)
            deal_damage_to_player(calc_damage(8,false));
        else
        {
            deal_damage_to_player(calc_damage(5,false));
            enemy.DEFENCE+=10;
        }
    }
    else if (round_count==transition_round)
    {
        deal_damage_to_player(calc_damage(16,false));
    }
    else
    {
        if ((round_count-transition_round)%2==1)
            enemy.DEFENCE+=10;
        else
            deal_damage_to_player(calc_damage(12,false));
    }
    round_count++;
    if (enemy.Poison>0)
    {
        enemy.HP-=enemy.Poison;
        if (enemy.HP<0)
            enemy.HP=0;
        enemy.Poison--;
    }

}

void show_enemy_intent(void)
{
    if (enemy_state==1)
    {
        sts_print("敌人意图（第 ", round_count, " 回合）：");
        if (round_count%3==1)
            sts_print("为自己增加 6 点格挡\n");
        else if (round_count%3==2)
            sts_print("对玩家造成 8 点伤害\n");
        else
            sts_print("对玩家造成 5 点伤害并为自己增加 10 点格挡\n");
    }
    else
    {
        sts_print("敌人意图（第 ", round_count, " 回合）：");
        if (round_count == transition_round)
            sts_print("对玩家造成 16 点伤害\n");
        else if ((round_count-transition_round)%2==1)
            sts_print("为自己增加 10 点格挡\n");
        else
            sts_print("对玩家造成 12 点伤害\n");
    }
}

int calc_damage(int base, bool attacker_is_player)
{
    if (attacker_is_player)
    {
        if (player.Weak)
            base=(base*75)/100; //先处理玩家是否虚弱，如果虚弱的话首先伤害减少
        if (enemy.Vulnerable)
            base=(base*150)/100; //之后再看敌人
    }
    else
    {
        if (enemy.Weak)
            base=(base*75)/100;
        if (player.Vulnerable)
            base=(base*150)/100;
    }
    return base;
}

void card_0(Card* card)
{
    deal_damage_to_enemy(calc_damage(card->damage,true));
}

void card_1(Card* card)
{
    player.DEFENCE +=5;
}

void card_2(Card* card)
{
    deal_damage_to_player(3);
    player.EP +=1;
}

void card_3(Card* card)
{
    deal_damage_to_enemy(calc_damage(card->damage,true));
    card->damage+=5;
}

void card_4(Card* card)
{
    deal_damage_to_enemy(calc_damage(card->damage,true));
    player.Weak=true;
}

void card_5(Card* card)
{
    enemy.Poison=5;
    player.Vulnerable=true;
}

void card_6(Card* card)
{
    if (player.Weak)
        enemy.Weak=true;
    if (player.Vulnerable)
        enemy.Vulnerable=true;
}

void test_logic(void) {
    int p, i;
    // 辅助清零宏（增加状态重置，保证测试隔离）
    #define CLEAR_ALL() do { \
        for (p = 0; p < 3; p++) { \
            for (i = 0; i < 11; i++) piles[p][i] = (Card){0}; \
            piles_count[p] = 0; \
        } \
        player.HP = 80; player.EP = 3; player.DEFENCE = 0; \
        enemy.HP = 40; enemy.DEFENCE = 0; \
        player.Vulnerable = false; player.Weak = false; \
        enemy.Vulnerable = false; enemy.Weak = false; enemy.Poison = 0; \
    } while(0)

    // ========== 1. 卡牌添加与结构体拷贝 ==========
    CLEAR_ALL();
    Card test_card = {"测试牌", 2, "描述", 0, 6};
    add_card_to_pile(test_card, 0);
    assert(piles_count[0] == 1);
    assert(strcmp(piles[0][0].name, "测试牌") == 0);
    assert(piles[0][0].cost == 2);
    assert(piles[0][0].type == 0);
    assert(piles[0][0].damage == 6);

    // ========== 2. 牌库初始化与洗牌 ==========
    CLEAR_ALL();
    reset_game();
    assert(piles_count[0] == 11);

    // ========== 3. 抽牌逻辑 ==========
    CLEAR_ALL();
    reset_game();
    draw_one_card(); draw_one_card(); draw_one_card();
    assert(piles_count[1] == 3);
    assert(piles_count[0] == 8);
    piles_count[1] = 5;
    int before_draw = piles_count[0];
    draw_one_card();
    assert(piles_count[1] == 5);
    assert(piles_count[0] == before_draw);

    // ========== 4. 弃牌堆→抽牌堆洗牌 ==========
    CLEAR_ALL();
    piles[2][0] = (Card){"A",1,"",0,6};
    piles[2][1] = (Card){"B",1,"",1,0};
    piles[2][2] = (Card){"C",0,"",2,0};
    piles_count[2] = 3;
    piles_count[0] = 0;
    draw_one_card();
    assert(piles_count[0] == 2);
    assert(piles_count[2] == 0);
    assert(piles_count[1] == 1);

    // ========== 5. 手牌移除 ==========
    CLEAR_ALL();
    piles[1][0] = (Card){"X",1,"",0,6};
    piles[1][1] = (Card){"Y",1,"",1,0};
    piles[1][2] = (Card){"Z",0,"",2,0};
    piles_count[1] = 3;
    reset_hand(2);
    assert(piles_count[1] == 2);
    assert(strcmp(piles[1][0].name, "X") == 0);
    assert(strcmp(piles[1][1].name, "Z") == 0);

    // ========== 6. 暴走效果 ==========
    CLEAR_ALL();
    player.EP = 3; enemy.HP = 40;
    Card rampage = {"暴走", 1, "", 3, 8};
    play_card(&rampage);
    assert(enemy.HP == 32);
    assert(rampage.damage == 13);
    play_card(&rampage);
    assert(enemy.HP == 19);
    assert(rampage.damage == 18);
    Card rampage2 = {"暴走", 1, "", 3, 8};
    play_card(&rampage2);
    assert(enemy.HP == 11);
    assert(rampage2.damage == 13);
    assert(rampage.damage == 18);

    // ========== 7. 打击/防御/放血 ==========
    CLEAR_ALL();
    player.EP = 3; enemy.HP = 40;
    Card strike = {"打击",1,"",0,6};
    play_card(&strike);
    assert(enemy.HP == 34);
    assert(player.EP == 2);

    CLEAR_ALL();
    player.EP = 3;
    Card defend = {"防御",1,"",1,0};
    play_card(&defend);
    assert(player.DEFENCE == 5);
    assert(player.EP == 2);

    CLEAR_ALL();
    player.HP = 80; player.EP = 2;
    Card blood = {"放血",0,"",2,0};
    play_card(&blood);
    assert(player.HP == 77);
    assert(player.EP == 3);

    // ====== 第六章测试 ======

    // 8. 敌人一阶段循环行为
    CLEAR_ALL();
    round_count = 1; enemy_state = 1; transition_round = 0;
    player.HP = 80; player.DEFENCE = 0;
    enemy.HP = 40; enemy.DEFENCE = 0;

    enemy_turn();
    assert(enemy.DEFENCE == 6);
    assert(player.HP == 80);
    assert(round_count == 2);

    enemy_turn();
    assert(player.HP == 72);
    assert(round_count == 3);

    enemy_turn();
    assert(player.HP == 67);
    assert(enemy.DEFENCE == 16);
    assert(round_count == 4);

    enemy_turn();
    assert(enemy.DEFENCE == 22);
    assert(player.HP == 67);
    assert(round_count == 5);

    // 9. 二阶段过渡与交替
    CLEAR_ALL();
    round_count = 3; enemy_state = 2; transition_round = 3;
    player.HP = 80; player.DEFENCE = 0;
    enemy.HP = 40; enemy.DEFENCE = 0;

    enemy_turn();
    assert(player.HP == 64);
    assert(round_count == 4);

    enemy_turn();
    assert(enemy.DEFENCE == 10);
    assert(player.HP == 64);
    assert(round_count == 5);

    enemy_turn();
    assert(player.HP == 52);
    assert(round_count == 6);

    enemy_turn();
    assert(enemy.DEFENCE == 20);
    assert(round_count == 7);

    // 10. 重置游戏后敌人状态恢复
    CLEAR_ALL();
    round_count = 5; enemy_state = 2; transition_round = 3;
    reset_game();
    assert(round_count == 1);
    assert(enemy_state == 1);
    assert(transition_round == 0);

    // ====== 扩展部分：新卡牌测试 ======

    // 11. 鲁莽（card_4）：12伤害，自己获得虚弱
    CLEAR_ALL();
    player.EP = 3; enemy.HP = 40;
    Card reckless = {"鲁莽", 1, "", 4, 12};
    play_card(&reckless);
    assert(enemy.HP == 28);           // 40 - 12
    assert(player.Weak == true);      // 自己获得虚弱

    // 12. 中毒（card_5）：敌人中毒5层，玩家获得易伤
    CLEAR_ALL();
    player.EP = 3; enemy.HP = 40;
    Card poison_card = {"中毒", 1, "", 5, 0};
    play_card(&poison_card);
    assert(enemy.Poison == 5);        // 敌人中毒5层
    assert(player.Vulnerable == true);// 玩家易伤

    // 13. 传染（card_6）：将玩家的虚弱和易伤复制给敌人
    CLEAR_ALL();
    player.Weak = true;
    player.Vulnerable = true;
    enemy.Weak = false;
    enemy.Vulnerable = false;
    enemy.Poison = 0;
    Card spread = {"传染", 0, "", 6, 0};
    play_card(&spread);
    assert(enemy.Weak == true);
    assert(enemy.Vulnerable == true);
    assert(enemy.Poison == 0);        // 不复制中毒

    // 14. 中毒伤害结算（直接扣血，无视护甲，减层）
    CLEAR_ALL();
    round_count = 1; enemy_state = 1; transition_round = 0;
    enemy.Poison = 5;
    enemy.HP = 40;
    enemy.DEFENCE = 10;               // 有护甲，但中毒直接扣血
    enemy_turn();                     // 触发加甲动作，然后中毒扣血
    assert(enemy.HP == 35);           // 40 - 5
    assert(enemy.DEFENCE == 16);      // 原护甲10 + 加甲6
    assert(enemy.Poison == 4);        // 层数减1

    // 15. 虚弱+易伤交互（玩家虚弱，敌人易伤）
    CLEAR_ALL();
    player.Weak = true;   // 玩家虚弱，伤害减少25%
    enemy.Vulnerable = true; // 敌人易伤，受到伤害增加50%
    enemy.HP = 40;
    Card strike2 = {"打击", 1, "", 0, 6};
    play_card(&strike2);
    // 基础伤害6 -> 虚弱后6*0.75=4(截断) -> 易伤后4*1.5=6
    assert(enemy.HP == 34); // 40 - 6

    // 16. 传染不传递中毒（确保中毒不被复制）
    CLEAR_ALL();
    player.Weak = true;
    player.Vulnerable = true;
    enemy.Poison = 3;       // 敌人已有中毒3层
    Card spread2 = {"传染", 0, "", 6, 0};
    play_card(&spread2);
    assert(enemy.Weak == true);
    assert(enemy.Vulnerable == true);
    assert(enemy.Poison == 3); // 中毒层数不变

    #undef CLEAR_ALL
}