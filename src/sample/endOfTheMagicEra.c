#include <ctoy.h>
//#include "../util/img_util.c"
#include "../util/img_utils.c"
#include <m_image.h>
#include <m_raster.h>
#include "../util/font.c"
#include <stdbool.h>
#include <math.h>
#include <m_math.h>

struct m_image window = M_IMAGE_IDENTITY();
struct m_image font_default = M_IMAGE_IDENTITY();
struct m_image game_objects = M_IMAGE_IDENTITY();
struct m_image background = M_IMAGE_IDENTITY();
struct m_image background_cutout = M_IMAGE_IDENTITY();
struct m_image cooldownbar = M_IMAGE_IDENTITY();
struct m_image enemy_cutout = M_IMAGE_IDENTITY();
struct m_image proj_cutout = M_IMAGE_IDENTITY();
struct m_image small_enemy = M_IMAGE_IDENTITY();
struct m_image medium_enemy = M_IMAGE_IDENTITY();
struct m_image large_enemy = M_IMAGE_IDENTITY();
struct m_image lavapool_icon = M_IMAGE_IDENTITY();

size_t time = 0;
float current_casting_time = 1;

char playername[20];
bool level_up_screen = false;

struct
{
   float x, y, hp;
   struct m_image image;
   size_t lvl, xp, maxhp, maxmana, width;
   float mana, speed, cooldown_multiplyer, damage_multiplyer;
   size_t spells[5]; // to save indexes of the listofspells
   // we still need to add classes (swordsman, archer, elemental wizard)
} player = {0, 0, 100, M_IMAGE_IDENTITY(), 0, 110, 500, 100, 32, 80.0, 1.0, 1.0, 1.0, {0, 0, 0, 0, 0}};

// idea: make a list of all types of enemies with its sprite ect, like the spelllist!

typedef struct enemy
{
   float hp;
   size_t type, range, xp_given, width;
   float x, y, damage, speed, cooldown;
   struct enemy *next;
} enemy_t;
// type is for enemies with projectiles and to differenciate the different textures
/* element weaknesses and strengths:
make enemies that have less or more damage done to themselves by some element
to differenciate between them, they could have different colored outlines
spelllist.element is needed though
*/

enemy_t enemylist = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};

typedef struct
{
   struct m_image projectile_image;
   float damage; // or hp, healpower, cooldownmultiplier (special)
   float mana;
   enum
   {
      ranged,
      explosive,
      heal,
      obstacle,
      melee,
      aoe,
      special,
      mana
   } type;
   float range; // duration
   // explosionrange = player size (special)
   // explosiondamage = damage multiplyer (special)
   float explosionrange, explosiondamage, explosionduration; // could be used for slow-effect on non explosives
   float speed;                                              // speed (special)
   float cooldown;
   size_t width; // for the hitbox
   char name[20];
} spell_t;

// array of all the spells, so that the playerslots just have to save the index of the owned spells
spell_t spelllist[] = {
    {M_IMAGE_IDENTITY(), 60, 4, ranged, 50, 0, 0, 0, 3, 40, 48, "basic attack"},           /* basic attack/ mana missile */
    {M_IMAGE_IDENTITY(), 50, 20, explosive, 50, 120, 25, 4, 2, 60, 32, "fireball"},        /* fireball */
    {M_IMAGE_IDENTITY(), 0, 15, explosive, 1, 80, 1, 60, 50, 30, 32, "embers"},            /* embers */
    {M_IMAGE_IDENTITY(), 2, 100, aoe, 400, 0, 0, 0, 0, 200, 200, "lava pool"},             /* lava pool */
    {M_IMAGE_IDENTITY(), 150, 12, ranged, 50, 0, 0, 0, 6, 30, 8, "air bullet"},            /* air bullet */
    {M_IMAGE_IDENTITY(), 0, 50, obstacle, 800, 0, 0, 0, 0, 50, 128, "earth wall"},         /* earthwall */
    {M_IMAGE_IDENTITY(), 1, 50, obstacle, 400, 0, 0, 0, 0, 60, 96, "thorn wall"},          /* thorn wall / thorn vines */
    {M_IMAGE_IDENTITY(), 0.2, 50, heal, 500, 0, 0, 0, 0, 200, 0, "regeneration"},          /* regeneration */
    {M_IMAGE_IDENTITY(), 50, 50, heal, 1, 0, 0, 0, 0, 100, 0, "heal"},                     /* heal */
    {M_IMAGE_IDENTITY(), 0.01, 5, heal, 1000, 0, 0, 0, 0, 40, 0, "cure"},                  /* cure */
    {M_IMAGE_IDENTITY(), 2, 30, aoe, 80, 0, 0, 0, 1.5, 40, 80, "cyclone"},                 /* cyclone / tornado */
    {M_IMAGE_IDENTITY(), 0.8, 30, special, 500, 0, 0, 0, 0, 100, 32, "mind acceleration"}, /* mind accereration blue */
    {M_IMAGE_IDENTITY(), 0, 20, special, 80, 0, 0, 0, 1, 80, 32, "tailwind"},              /* tailwind white */
    {M_IMAGE_IDENTITY(), 0.5, 100, special, 600, 0, 1.5, 0, 0, 200, 32, "overcharge"},     /* overcharge yellow */
    {M_IMAGE_IDENTITY(), 0, 150, special, 1000, 1, 2, 0, 1, 300, 32, "giantification"},    /* giantification red */
    {M_IMAGE_IDENTITY(), 4, 40, melee, 250, 0, 0, 0, 2, 83, 32, "wind shield"},            /* whirlwind or smth */
    {M_IMAGE_IDENTITY(), 40, 120, melee, 200, 0, 0, 0, 2, 350, 32, "whirlwind"},           /* whirlwind or smth */
    {M_IMAGE_IDENTITY(), 2, 30, mana, 25, 0, 0, 0, 0, 150, 32, "mana forge"},              /* mana forge */
    {M_IMAGE_IDENTITY(), 0.1, 20, mana, 400, 0, 0, 0, 0, 100, 32, "recovery"},
    {M_IMAGE_IDENTITY(), 4, 2, mana, 1, 0, 0, 0, 0, 15, 32, "meditation"}

    // pushing wind (add the univector of the projectile * damage to the position of enemy)
    // nuke (full screen explotion, very slow projectile, high cooldown)
    // freeze (=freezing an enemy in place) or binding vines
    // maybe: ice, earth, fire, wind shield (on top of hp)
    // some sound spell (wind element, huge aoe, long cooldown, damage in % (maybe))
    // ice spike (low cooldown)
    // volcano (huge wall with damage to those touching it)
    // maybe: elemental incarnation (=more damage in one element for some time; an enum in spelllist is needed)
    // mana forge (=get mana, but long cooldown)
    // overcharging or giantification (=playersize, mana regen, hp, damage, speed gets doubled, but at the cost of mana / xp / hp)
    // blizzard (enemies are slowed)
    // energy syphon (you loose hp/xp (or speed/damage for some time) but get mp) or (make enemy smaller, get mana (if already really small, he gets faster))
    // alternative spells to get mana

    // Teleport to mouse/tp range in the direction of mouse
    // Tornado: moves around the map randomly (up, down, left, right, diagonals)
    // wave/firewall depending on angle of mouse "wall" moving in that direction dealing damage or pushing touched enemys
    // Black Hole: teleports to random position until duration = 0, kills all enemies it touches
};

typedef struct projectile
{
   float x, y;
   double x_orientation, y_orientation; // also used for angle in meele
   size_t width, spellID;
   float damage, speed, duration_left;
   struct projectile *next;
} proj_t;
// mybe add piercing --> how many enemies can get hit before destruction

proj_t projectilelist = {0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};

void spawn_enemies()
{
   int amount = m_rand() % (int)(time / 10000 + 2) + 1;

   enemy_t *new = enemylist.next;
   for (int i = 0; i < amount; ++i)
   {
      enemylist.next = malloc(sizeof(enemy_t));
      enemylist.next->next = new;
      new = enemylist.next;
      new->type = m_rand() % 3 + 1;
      size_t lvl = time / 3000;

      if (new->type == 3)
      {
         new->speed = 0.3 + 0.02 * lvl;
         new->hp = 1000 + 30 * pow(1.5, lvl);
         new->range = 0;
         new->damage = 1;
         new->width = 128;
         new->xp_given = 30 * pow(1.2, lvl);
      }
      else if (new->type == 2)
      {
         new->speed = 0.5 + 0.05 * lvl;
         new->hp = 300 + 10 * pow(1.5, lvl);
         new->range = 0;
         new->damage = 0.5;
         new->width = 64;
         new->xp_given = 20 * pow(1.2, lvl);
      }
      else
      {
         new->speed = 1 + 0.1 * lvl;
         new->hp = 50 + pow(1.5, lvl);
         new->range = 0;
         new->damage = 0.2;
         new->width = 32;
         new->xp_given = 10 * pow(1.2, lvl);
      }

      short pos = m_rand() % 4; // 0 = top; 1 = bottom; 2 = left; 3 = right
      switch (pos)
      {
      case 0:
         new->y = player.y - (new->width + 2);
         break;
      case 1:
         new->y = player.y + window.height + (new->width + 2);
         break;
      case 2:
         new->x = player.x - (new->width + 2);
         break;
      case 3:
         new->x = player.x + window.width + (new->width + 2);
      }
      if (pos < 2)
      {
         new->x = player.x + m_randf() * window.width;
      }
      else
      {
         new->y = player.y + m_randf() * window.height;
      }
   }
}

void castprojectile(size_t spellslot)
{
   // listofspells[player.spells[spellslot]]
   spell_t spell = spelllist[player.spells[spellslot]];
   if (player.mana >= spell.mana)
   {
      player.mana -= spell.mana;
      proj_t *newprojectile = &projectilelist;
      while (newprojectile->next != NULL)
         newprojectile = newprojectile->next;
      newprojectile->next = malloc(sizeof(proj_t));
      newprojectile = newprojectile->next;
      newprojectile->next = NULL;

      newprojectile->x = player.x + window.width / 2;
      newprojectile->y = player.y + window.height / 2;
      if (spell.speed != 0)
      {
         double temproot = 1 / sqrt((ctoy_mouse_x() - 0.5) * (ctoy_mouse_x() - 0.5) + (ctoy_mouse_y() - 0.5) / 256 * 81 * (ctoy_mouse_y() - 0.5));
         newprojectile->x_orientation = temproot * ((ctoy_mouse_x() - 0.5));
         newprojectile->y_orientation = temproot * ((ctoy_mouse_y() - 0.5) / 16 * 9);
      }
      if (spell.type == mana || spell.type == heal)
      {
         newprojectile->damage = spell.damage;
      }
      else
      {
         newprojectile->damage = spell.damage * player.damage_multiplyer;
      }
      newprojectile->speed = spell.speed;
      newprojectile->duration_left = spell.range;
      newprojectile->width = spell.width * player.width / 32;
      newprojectile->spellID = player.spells[spellslot];
      if (spell.type == special)
      {
         newprojectile->damage = spell.damage;
         player.cooldown_multiplyer += spell.damage;
         if (player.width + 32 * spell.explosionrange > window.height)
         {
            newprojectile->x_orientation = 0;
         }
         else
         {
            newprojectile->x_orientation = spell.explosionrange;
            player.width += spell.explosionrange * 32;
         }
         player.speed += spell.speed;
         player.damage_multiplyer += spell.explosiondamage;
         m_image_resize(&player.image, &player.image, player.width, player.width);
      }
   }
}

float2 getvector(float ax, float ay, float bx, float by)
{
   float2 vect = {ax - bx, ay - by};
   return vect;
}

float2 unitvector(float2 vect)
{
   float help = 1 / (sqrt(vect.x * vect.x + vect.y * vect.y));
   float2 unitvector = {help * vect.x, help * vect.y};
   return unitvector;
}

bool checkcollision(float x1, float y1, float w1, float x2, float y2, float w2)
{
   float2 vect = getvector(x1, y1, x2, y2);
   if (sqrt(vect.x * vect.x + vect.y * vect.y) < w1 / 2 + w2 / 2)
      return true;
   return false;
}

void update_enemies()
{
   // m_rand()
   if (time % 300 == 0)
      spawn_enemies();
   enemy_t *prevtemp = &enemylist;
   for (enemy_t *temp = enemylist.next; temp != NULL; temp = temp->next)
   {
      if (temp->hp <= 0)
      {
         // remove enemy from list
         player.xp += temp->xp_given;
         prevtemp->next = temp->next;
         enemy_t *cache = temp;
         temp = prevtemp; // after continue, temp gets set to temp->next
         free(cache);
         continue;
      }
      // movement towards player
      float savex = temp->x;
      float savey = temp->y;
      float2 vector = {player.x + (float)(window.width / 2 - temp->x), player.y + (float)(window.height / 2) - temp->y};
      float root = 1.0F / sqrt(vector.x * vector.x + vector.y * vector.y);
      temp->x += vector.x * root * temp->speed;
      temp->y += vector.y * root * temp->speed;

      for (proj_t *proj = projectilelist.next; proj != NULL; proj = proj->next)
      {
         if (spelllist[proj->spellID].type == obstacle)
         {
            if (checkcollision(proj->x, proj->y, proj->width, temp->x, temp->y, temp->width))
            {
               temp->x = savex;
               temp->y = savey;
               proj->duration_left -= temp->damage;
               temp->hp -= proj->damage;
               break;
            }
         }
      }
      if (temp->x + temp->width / 2 - 1 > player.x && temp->x - temp->width / 2 + 1 < player.x + window.width && temp->y + temp->width / 2 - 1 > player.y && temp->y - temp->width / 2 + 1 < player.y + window.height)
      {
         if (temp->type == 1)
            m_image_resize(&enemy_cutout, &small_enemy, temp->width, temp->width);
         else if (temp->type == 2)
            m_image_resize(&enemy_cutout, &medium_enemy, temp->width, temp->width);
         else if (temp->type == 3)
            m_image_resize(&enemy_cutout, &large_enemy, temp->width, temp->width);
         // game crashes because enemy gets here when it should not be here!
         float enemysize[] = {0, 0, temp->width, temp->width};
         if (temp->x - temp->width / 2 < player.x)
         {
            enemysize[0] = player.x + temp->width / 2 - temp->x;
            enemysize[2] -= player.x + temp->width / 2 - temp->x;
         }
         else if (temp->x + temp->width / 2 > player.x + window.width)
         {
            enemysize[2] -= temp->x + temp->width / 2 - player.x - window.width;
         }
         if (temp->y - temp->width / 2 < player.y)
         {
            enemysize[1] = player.y + temp->width / 2 - temp->y;
            enemysize[3] -= player.y + temp->width / 2 - temp->y;
         }
         else if (temp->y + temp->width / 2 > player.y + window.height)
         {
            enemysize[3] -= temp->y + temp->width / 2 - player.y - window.height;
         }
         m_image_copy_sub_image(&enemy_cutout, &enemy_cutout, enemysize[0], enemysize[1], enemysize[2], enemysize[3]);

         m_image_draw(&window, &enemy_cutout, temp->x - temp->width / 2 + enemysize[0] - player.x, temp->y - temp->width / 2 + enemysize[1] - player.y);
      }
      prevtemp = temp;
   }
}

void cometDraw(float angle, int radius, int circleAm, proj_t *temp)
{
   float2 help;
   for (int i = 1; i <= circleAm; ++i)
   {
      if (i == circleAm)
      {
         temp->x = player.x + cos(angle) * radius + window.width / 2;
         temp->y = player.y + sin(angle) * radius + window.height / 2;
      }
      angle += 0.25;
      help.x = cos(angle) * radius;
      help.y = sin(angle) * radius;
      m_image_resize(&proj_cutout, &spelllist[temp->spellID].projectile_image, spelllist[temp->spellID].projectile_image.width * i / circleAm, spelllist[temp->spellID].projectile_image.height * i / circleAm);
      if (help.x + window.width / 2 - proj_cutout.width / 2 > 0 && help.x + window.width / 2 - proj_cutout.width / 2 < window.width + 32 && help.y + window.height / 2 - proj_cutout.height / 2 > 0 && help.y + window.height / 2 - proj_cutout.height / 2 + 32 < window.height)
      {
         m_image_draw(&window, &proj_cutout, help.x + window.width / 2 - proj_cutout.width / 2, help.y + window.height / 2 - proj_cutout.height / 2);
      }
   }
}

void update_projectiles()
{
   proj_t *prevtemp = &projectilelist;
   for (proj_t *temp = projectilelist.next; temp != NULL; temp = temp->next)
   {
      if (temp->duration_left <= 0)
      {
         if (spelllist[temp->spellID].type == explosive)
         {
            temp->damage = spelllist[temp->spellID].explosiondamage * player.damage_multiplyer;
            temp->duration_left = spelllist[temp->spellID].explosionduration;
            temp->speed = 0;
            temp->width = spelllist[temp->spellID].explosionrange * player.width / 32;
            temp->spellID = 3;
         }
         else
         {
            if (spelllist[temp->spellID].type == special)
            {
               // remove the buffs given to the player by the spell
               player.cooldown_multiplyer -= temp->damage;
               player.width -= temp->x_orientation * 32;
               player.speed -= temp->speed;
               player.damage_multiplyer -= spelllist[temp->spellID].explosiondamage;
               m_image_copy_sub_image(&player.image, &game_objects, 32, 32, 32, 32);
               m_image_resize(&player.image, &player.image, player.width, player.width);
            }
            // remove projectile from list
            prevtemp->next = temp->next;
            proj_t *cache = temp;
            temp = prevtemp;
            free(cache);
            continue;
         }
      }
      if (spelllist[temp->spellID].type != obstacle)
         temp->duration_left--;
      if (spelllist[temp->spellID].type == heal)
      {
         player.hp += temp->damage;
         if (player.hp > player.maxhp)
            player.hp = player.maxhp;
      }
      else if (spelllist[temp->spellID].type == special)
      {
         m_image_resize(&proj_cutout, &spelllist[temp->spellID].projectile_image, player.width, player.width);
         m_image_draw(&window, &proj_cutout, window.width / 2 - proj_cutout.width / 2, window.height / 2 - proj_cutout.height / 2);
      }
      else if (spelllist[temp->spellID].type == mana)
      {
         player.mana += temp->damage;
      }
      else if (temp->spellID == 15)
      {
         int circleAm = 5;
         int radius = 80;
         float2 where;
         cometDraw(temp->x_orientation, radius, circleAm, temp);
         temp->x_orientation += (float)(temp->speed / 20.0);
         if (temp->x_orientation >= 360)
         {
            temp->x_orientation -= 360;
         }
      }
      else if (temp->spellID == 16)
      {
         int cometAm = 3;
         int circleAm = 5;

         for (int i = 0; i < cometAm; ++i)
         {
            temp->x_orientation += 0.3;
            temp->y_orientation += 2 * M_PI / cometAm;
            cometDraw(temp->y_orientation, temp->x_orientation, circleAm, temp);
            temp->y_orientation += (float)(temp->speed / 20.0);
            if (temp->y_orientation >= 2 * M_PI)
            {
               temp->y_orientation -= 2 * M_PI;
            }
         }
      }
      else
      {
         temp->x += temp->x_orientation * temp->speed;
         temp->y += temp->y_orientation * temp->speed;
         if (temp->x + temp->width / 2 - 1 > player.x && temp->x - temp->width / 2 + 1 < player.x + window.width && temp->y + temp->width / 2 - 1 > player.y && temp->y - temp->width / 2 + 1 < player.y + window.height)
         {
            m_image_resize(&proj_cutout, &spelllist[temp->spellID].projectile_image, temp->width, temp->width);
            float projsize[] = {0, 0, temp->width, temp->width};
            if (temp->x - temp->width / 2 < player.x)
            {
               projsize[0] = player.x + temp->width / 2 - temp->x;
               projsize[2] -= player.x + temp->width / 2 - temp->x;
            }
            if (temp->x + temp->width / 2 > player.x + window.width)
            {
               projsize[2] -= temp->x + temp->width / 2 - player.x - window.width;
            }
            if (temp->y - temp->width / 2 < player.y)
            {
               projsize[1] = player.y + temp->width / 2 - temp->y;
               projsize[3] -= player.y + temp->width / 2 - temp->y;
            }
            if (temp->y + temp->width / 2 > player.y + window.height)
            {
               projsize[3] -= temp->y + temp->width / 2 - player.y - window.height;
            }
            m_image_copy_sub_image(&proj_cutout, &proj_cutout, projsize[0], projsize[1], projsize[2], projsize[3]);

            m_image_draw(&window, &proj_cutout, temp->x - temp->width / 2 + projsize[0] - player.x, temp->y - temp->width / 2 + projsize[1] - player.y);
         }
      }
      prevtemp = temp;
   }
}

void collisionfind()
{
   if (enemylist.next != NULL && projectilelist.next != NULL)
   {
      for (enemy_t *enemy = enemylist.next; enemy != NULL; enemy = enemy->next)
      {
         proj_t *prevproj = &projectilelist;
         for (proj_t *proj = projectilelist.next; proj != NULL; proj = proj->next)
         {
            if (spelllist[proj->spellID].type != special && spelllist[proj->spellID].type != heal && spelllist[proj->spellID].type != obstacle)
            {
               if (checkcollision(proj->x, proj->y, proj->width, enemy->x, enemy->y, enemy->width))
               {
                  enemy->hp -= proj->damage;
                  if (spelllist[proj->spellID].type == explosive)
                  {
                     proj->damage = spelllist[proj->spellID].explosiondamage * player.damage_multiplyer;
                     proj->duration_left = spelllist[proj->spellID].explosionduration;
                     proj->speed = 0;
                     proj->width = spelllist[proj->spellID].explosionrange * player.width / 32;
                     proj->spellID = 3;
                  }
                  else if (spelllist[proj->spellID].type != aoe && spelllist[proj->spellID].type != melee)
                  {
                     proj_t *cache = proj;
                     prevproj->next = proj->next;
                     proj = prevproj;
                     free(cache);
                  }
               }
            }
            prevproj = proj;
         }
      }
   }

   for (enemy_t *enemy = enemylist.next; enemy != NULL; enemy = enemy->next)
   {
      if (checkcollision(enemy->x, enemy->y, enemy->width, player.x + window.width / 2, player.y + window.height / 2, player.width))
      {
         player.hp -= enemy->damage;
      }
   }
}

void update_player()
{
   // make a xp-bar or at least show how much you still need
   if (player.xp >= (10 * pow(1.2, player.lvl) + 100))
   {
      player.xp -= (10 * pow(1.2, player.lvl) + 100);
      ++player.lvl;
      player.maxmana += 10;
      level_up_screen = true;
   }
   if (time % 5 == 0)
      player.mana++;
   if (player.mana > player.maxmana)
   {
      player.mana = player.maxmana;
   }

   m_image_draw(&window, &player.image, window.width / 2 - player.width / 2, window.height / 2 - player.width / 2);
   m_image_draw(&window, &cooldownbar, window.width / 2 - player.image.width / 2, window.height / 2 - player.image.height / 2);

   float2 walkvector = {0, 0};
   if (ctoy_key_pressed(CTOY_KEY_W) /* && player.x < window.width - player.image.width*/)
   {
      walkvector.y--;
   }
   if (ctoy_key_pressed(CTOY_KEY_S) /* && player.x > 0*/)
   {
      walkvector.y++;
   }
   if (ctoy_key_pressed(CTOY_KEY_A) /* && player.y > 0*/)
   {
      walkvector.x--;
   }
   if (ctoy_key_pressed(CTOY_KEY_D) /* && player.y < window.height - player.image.height*/)
   {
      walkvector.x++;
   }
   if (walkvector.x != 0 && walkvector.y != 0)
   {
      player.x += walkvector.x * 0.71 * player.speed;
      player.y += walkvector.y * 0.71 * player.speed;
   }
   else if (walkvector.x != 0)
   {
      player.x += walkvector.x * player.speed;
   }
   else if (walkvector.y != 0)
   {
      player.y += walkvector.y * player.speed;
   }

   if (ctoy_mouse_button_pressed(CTOY_MOUSE_BUTTON_LEFT))
   {
      if (current_casting_time >= spelllist[player.spells[0]].cooldown)
      {
         castprojectile(0);
         current_casting_time = 0;
      }
      else
      {
         m_image_resize(&cooldownbar, &cooldownbar, 1 + (float)player.image.width * (float)(1.0 - (float)current_casting_time / spelllist[player.spells[0]].cooldown), cooldownbar.height);
         current_casting_time += player.cooldown_multiplyer;
      }
   }
   else if (ctoy_mouse_button_pressed(CTOY_MOUSE_BUTTON_RIGHT))
   {
      if (current_casting_time >= spelllist[player.spells[1]].cooldown)
      {
         castprojectile(1);
         current_casting_time = 1;
      }
      else
      {
         m_image_resize(&cooldownbar, &cooldownbar, 1 + (float)player.image.width * (float)(1.0 - (float)current_casting_time / spelllist[player.spells[1]].cooldown), cooldownbar.height);
         current_casting_time += player.cooldown_multiplyer;
      }
   }
   else if (ctoy_mouse_button_pressed(CTOY_MOUSE_BUTTON_MIDDLE) || ctoy_key_pressed(CTOY_KEY_1))
   {
      if (current_casting_time >= spelllist[player.spells[2]].cooldown)
      {
         castprojectile(2);
         current_casting_time = 1;
      }
      else
      {
         m_image_resize(&cooldownbar, &cooldownbar, 1 + (float)player.image.width * (float)(1.0 - (float)current_casting_time / spelllist[player.spells[2]].cooldown), cooldownbar.height);
         current_casting_time += player.cooldown_multiplyer;
      }
   }
   else if (ctoy_mouse_button_pressed(CTOY_MOUSE_BUTTON_4) || ctoy_key_pressed(CTOY_KEY_2))
   {
      if (current_casting_time >= spelllist[player.spells[3]].cooldown)
      {
         castprojectile(3);
         current_casting_time = 1;
      }
      else
      {
         m_image_resize(&cooldownbar, &cooldownbar, 1 + (float)player.image.width * (float)(1.0 - (float)current_casting_time / spelllist[player.spells[3]].cooldown), cooldownbar.height);
         current_casting_time += player.cooldown_multiplyer;
      }
   }
   else if (ctoy_mouse_button_pressed(CTOY_MOUSE_BUTTON_5) || ctoy_key_pressed(CTOY_KEY_3))
   {
      if (current_casting_time >= spelllist[player.spells[4]].cooldown)
      {
         castprojectile(4);
         current_casting_time = 1;
      }
      else
      {
         m_image_resize(&cooldownbar, &cooldownbar, 1 + (float)player.image.width * (float)(1.0 - (float)current_casting_time / spelllist[player.spells[4]].cooldown), cooldownbar.height);
         current_casting_time += player.cooldown_multiplyer;
      }
   }
   else
   {
      current_casting_time = 1;
      m_image_resize(&cooldownbar, &cooldownbar, 1, cooldownbar.height);
   }
}

void pixel_init(void)
{
   for (int i = 0; i < (window.width * window.height * 3); i++)
   {
      ((float *)(window.data))[i] = 0.0;
   }
}

void refresh_background()
{
   // needs a png that is at least the size of the screen
   float2 screenscroll = {player.x, player.y};
   while (screenscroll.x < 0)
      screenscroll.x += 10 * background.width;
   while (screenscroll.y < 0)
      screenscroll.y += 10 * background.height;
   screenscroll.x = (int)screenscroll.x % background.width;
   screenscroll.y = (int)screenscroll.y % background.height;

   int temp[2] = {background.width - screenscroll.x, background.height - screenscroll.y};
   if (temp[0] < window.width && temp[1] < window.height)
   {
      m_image_copy_sub_image(&background_cutout, &background, screenscroll.x, screenscroll.y, temp[0], temp[1]);
      m_image_draw(&window, &background_cutout, 0, 0);
      m_image_copy_sub_image(&background_cutout, &background, 0, screenscroll.y, window.width - temp[0], temp[1]);
      m_image_draw(&window, &background_cutout, temp[0], 0);
      m_image_copy_sub_image(&background_cutout, &background, screenscroll.x, 0, temp[0], window.height - temp[1]);
      m_image_draw(&window, &background_cutout, 0, temp[1]);
      m_image_copy_sub_image(&background_cutout, &background, 0, 0, window.width - temp[0], window.height - temp[1]);
      m_image_draw(&window, &background_cutout, temp[0], temp[1]);
   }
   else if (temp[0] < window.width)
   {
      m_image_copy_sub_image(&background_cutout, &background, screenscroll.x, screenscroll.y, temp[0], window.height);
      m_image_draw(&window, &background_cutout, 0, 0);
      m_image_copy_sub_image(&background_cutout, &background, 0, screenscroll.y, window.width - temp[0], window.height);
      m_image_draw(&window, &background_cutout, temp[0], 0);
   }
   else if (temp[1] < window.height)
   {
      m_image_copy_sub_image(&background_cutout, &background, screenscroll.x, screenscroll.y, window.width, temp[1]);
      m_image_draw(&window, &background_cutout, 0, 0);
      m_image_copy_sub_image(&background_cutout, &background, screenscroll.x, 0, window.width, window.height - temp[1]);
      m_image_draw(&window, &background_cutout, 0, temp[1]);
   }
   else
   {
      m_image_copy_sub_image(&background_cutout, &background, screenscroll.x, screenscroll.y, window.width, window.height);
      m_image_draw(&window, &background_cutout, 0, 0);
   }
}

void refresh_gui()
{
   float color[] = {0.0, 0.0, 1.0};
   char mana[11] = {'\0'};
   float *pixel = (float *)window.data;
   for (int i = 0; i < 5; i++)
   {
      for (int y = 20 + 40 * i; y < 40 + 40 * i; y++)
      {
         for (int x = 20; x < 40; x++)
         {
            pixel[(y * window.width + x) * 3] = 1.0;     // R
            pixel[(y * window.width + x) * 3 + 1] = 1.0; // G
            pixel[(y * window.width + x) * 3 + 2] = 1.0; // B
         }
      }
      if (player.spells[i] == 3)
         m_image_draw(&window, &lavapool_icon, 22, 40 * i + 22);
      else
      {
         m_image_resize(&proj_cutout, &spelllist[player.spells[i]].projectile_image, 16, 16);
         m_image_draw(&window, &proj_cutout, 22, 40 * i + 22);
      }
      sprintf(mana, "%.0f", spelllist[player.spells[i]].mana);
      m_font_draw(&window, &font_default, mana, strlen(mana), 15, 40 * i + 40, color);
   }
   char health[9] = {'\0'};
   char lvl[9] = {'\0'};
   char xp[13] = {'\0'};
   sprintf(health, "HP:%.0f", player.hp);
   sprintf(lvl, "Lvl:%d", player.lvl);
   sprintf(xp, "XP:%d", player.xp);
   sprintf(mana, "MP:%.0f", player.mana);
   m_font_draw(&window, &font_default, mana, strlen(mana), window.width - 100, window.height - 80, color);
   color[0] = 1.0;
   color[2] = 0.0;
   m_font_draw(&window, &font_default, health, strlen(health), window.width - 100, window.height - 60, color);
   color[0] = 0.0;
   color[1] = 1.0;
   m_font_draw(&window, &font_default, lvl, strlen(lvl), window.width - 100, window.height - 40, color);
   m_font_draw(&window, &font_default, xp, strlen(xp), window.width - 100, window.height - 20, color);
}

void ctoy_begin(void)
{
   printf("Gib deinen Namen ein: ");
   scanf("%s", playername);

   ctoy_window_size(960, 540);
   ctoy_window_title("End of the Magic Era");

   m_image_create(&window, M_FLOAT, 480, 270, 3);
   m_image_load_float(&font_default, "data/font/default.tga");
   m_image_load_float(&game_objects, "data/GameObjects-eotme.png");
   m_image_load_float(&background, "data/background-eotme.png");

   m_image_copy_sub_image(&spelllist[0].projectile_image, &game_objects, 32, 0, 32, 32);
   m_image_copy_sub_image(&spelllist[1].projectile_image, &game_objects, 0, 0, 32, 32);
   m_image_copy_sub_image(&spelllist[2].projectile_image, &game_objects, 0, 0, 32, 32);
   m_image_load_float(&spelllist[3].projectile_image, "data/lavapool-eotme.png");
   m_image_resize(&lavapool_icon, &spelllist[3].projectile_image, 16, 16);
   m_image_copy_sub_image(&spelllist[4].projectile_image, &game_objects, 64, 0, 32, 32);
   m_image_copy_sub_image(&spelllist[5].projectile_image, &game_objects, 64, 0, 32, 32);
   m_image_load_float(&spelllist[6].projectile_image, "data/Thornwall-eotme.png");
   m_image_load_float(&spelllist[7].projectile_image, "data/heal-eotme.png");
   m_image_copy(&spelllist[8].projectile_image, &spelllist[7].projectile_image);
   m_image_copy(&spelllist[9].projectile_image, &spelllist[7].projectile_image);
   m_image_load_float(&spelllist[10].projectile_image, "data/cyclone-eotme.png");
   m_image_copy_sub_image(&spelllist[11].projectile_image, &game_objects, 64, 32, 32, 32);
   m_image_copy_sub_image(&spelllist[12].projectile_image, &game_objects, 160, 32, 32, 32);
   m_image_copy_sub_image(&spelllist[13].projectile_image, &game_objects, 96, 32, 32, 32);
   m_image_copy_sub_image(&spelllist[14].projectile_image, &game_objects, 128, 32, 32, 32);
   m_image_copy_sub_image(&spelllist[15].projectile_image, &game_objects, 64, 0, 32, 32);
   m_image_copy_sub_image(&spelllist[16].projectile_image, &game_objects, 64, 0, 32, 32);

   m_image_copy_sub_image(&player.image, &game_objects, 32, 32, 32, 32);

   m_image_resize(&spelllist[17].projectile_image, &player.image, 16, 16);
   m_image_copy(&spelllist[18].projectile_image, &spelllist[17].projectile_image);
   m_image_copy(&spelllist[19].projectile_image, &spelllist[17].projectile_image);

   m_image_copy_sub_image(&small_enemy, &game_objects, 0, 32, 32, 32);
   m_image_copy_sub_image(&medium_enemy, &game_objects, 0, 64, 64, 64);
   m_image_copy_sub_image(&large_enemy, &game_objects, 64, 64, 128, 128);

   m_image_create(&cooldownbar, M_FLOAT, 1, 5, 3);
   float *pixel = (float *)cooldownbar.data;
   for (int i = 0; i < cooldownbar.size / cooldownbar.comp; i++, pixel += 3)
   {
      pixel[0] = 1.0;
   }
}
void ctoy_main_loop(void)
{
   // show the 5 spells you got in the top right corner
   // when lvlup, show 3 options on top (the game won't stop). if you choose a spell, you can click on your spells on the right
   if (player.hp > 0 && level_up_screen == false)
   {
      time++;

      refresh_background();
      collisionfind();
      update_projectiles();
      update_enemies();
      update_player();
      refresh_gui();
   }
   else if (level_up_screen)
   {
      static size_t random_upgrade[2];
      static size_t random[3];
      static bool drawonce = false;
      if (!drawonce)
      {
         // decise what the /spells are here
         random[0] = m_rand() % 20;
         while (random[0] == player.spells[0] || random[0] == player.spells[1] || random[0] == player.spells[2] || random[0] == player.spells[3] || random[0] == player.spells[4])
            random[0] = m_rand() % 20;
         if (player.lvl < 6)
         {
            random[1] = m_rand() % 20;
            while (random[1] == player.spells[0] || random[1] == player.spells[1] || random[1] == player.spells[2] || random[1] == player.spells[3] || random[1] == player.spells[4] || random[1] == random[0])
               random[1] = m_rand() % 20;
            random[2] = m_rand() % 20;
            while (random[2] == player.spells[0] || random[2] == player.spells[1] || random[2] == player.spells[2] || random[2] == player.spells[3] || random[2] == player.spells[4] || random[2] == random[0] || random[2] == random[1])
               random[2] = m_rand() % 20;
         }
         else
         {
            random[1] = player.spells[m_rand() % 5];
            random[2] = player.spells[m_rand() % 5];
         }

         float *pixel = (float *)window.data;
         for (int y = window.height / 7; y < window.height * 2 / 7; y++)
         {
            for (int x = 100; x < window.width - 100; x++)
            {
               pixel[(y * window.width + x) * 3] = 0.0;     // R
               pixel[(y * window.width + x) * 3 + 1] = 0.0; // G
               pixel[(y * window.width + x) * 3 + 2] = 0.0; // B
            }
         }
         for (int y = window.height * 3 / 7; y < window.height * 4 / 7; y++)
         {
            for (int x = 100; x < window.width - 100; x++)
            {
               pixel[(y * window.width + x) * 3] = 0.0;     // R
               pixel[(y * window.width + x) * 3 + 1] = 0.0; // G
               pixel[(y * window.width + x) * 3 + 2] = 0.0; // B
            }
         }
         for (int y = window.height * 5 / 7; y < window.height * 6 / 7; y++)
         {
            for (int x = 100; x < window.width - 100; x++)
            {
               pixel[(y * window.width + x) * 3] = 0.0;     // R
               pixel[(y * window.width + x) * 3 + 1] = 0.0; // G
               pixel[(y * window.width + x) * 3 + 2] = 0.0; // B
            }
         }
         float color[3] = {1.0, 1.0, 1.0};
         m_image_resize(&proj_cutout, &spelllist[random[0]].projectile_image, 32, 32);
         m_image_draw(&window, &proj_cutout, 105, window.height / 7 + 5);
         m_font_draw(&window, &font_default, spelllist[random[0]].name, 15, 140, window.height / 7 + 5, color);
         m_image_resize(&proj_cutout, &spelllist[random[1]].projectile_image, 32, 32);
         m_image_draw(&window, &proj_cutout, 105, window.height * 3 / 7 + 5);
         m_font_draw(&window, &font_default, spelllist[random[1]].name, 15, 140, window.height * 3 / 7 + 5, color);
         m_image_resize(&proj_cutout, &spelllist[random[2]].projectile_image, 32, 32);
         m_image_draw(&window, &proj_cutout, 105, window.height * 5 / 7 + 5);
         m_font_draw(&window, &font_default, spelllist[random[2]].name, 15, 140, window.height * 5 / 7 + 5, color);
         if (player.lvl > 5)
         {
            char upgrade_string[20];
            random_upgrade[0] = m_rand() % 3; // damage, mana or cooldown buff
            random_upgrade[1] = m_rand() % 3;

            // what about special spells
            if (spelllist[random[1]].type == special)
            {
               random_upgrade[0] = m_rand() % 6; // cooldown_multiplier, mana, cooldown, duration, damga_multiplier, speed
               if (random_upgrade[0] == 0)
                  strcpy(upgrade_string, "+10% casting speed");
               else if (random_upgrade[0] == 1)
                  strcpy(upgrade_string, "-20% cost");
               else if (random_upgrade[0] == 2)
                  strcpy(upgrade_string, "-10% cooldown");
               else if (random_upgrade[0] == 3)
                  strcpy(upgrade_string, "+10% duration");
               else if (random_upgrade[0] == 4)
                  strcpy(upgrade_string, "+10% damage");
               else if (random_upgrade[0] == 5)
                  strcpy(upgrade_string, "+10% speed");
            }
            else
            {
               if (spelllist[random[1]].type == aoe || spelllist[random[1]].type == explosive)
               {
                  random_upgrade[0] = m_rand() % 5; // damage, mana, cooldown, duration, size
                  // explosiondamage, mana, cooldown, explosionduration, explosionsize
               }
               if (random_upgrade[0] == 0)
                  strcpy(upgrade_string, "+10% damage");
               else if (random_upgrade[0] == 1)
                  strcpy(upgrade_string, "-20% cost");
               else if (random_upgrade[0] == 2)
                  strcpy(upgrade_string, "-10% cooldown");
               else if (random_upgrade[0] == 3)
                  strcpy(upgrade_string, "+10% duration");
               else if (random_upgrade[0] == 4)
                  strcpy(upgrade_string, "+30% size");
            }
            m_font_draw(&window, &font_default, upgrade_string, 15, 140, window.height * 3 / 7 + 20, color);
            if (spelllist[random[2]].type == special)
            {
               random_upgrade[1] = m_rand() % 6; // cooldown_multiplier, mana, cooldown, duration, damga_multiplier, speed
               if (random_upgrade[1] == 0)
                  strcpy(upgrade_string, "+10% casting speed");
               else if (random_upgrade[1] == 1)
                  strcpy(upgrade_string, "-20% cost");
               else if (random_upgrade[1] == 2)
                  strcpy(upgrade_string, "-10% cooldown");
               else if (random_upgrade[1] == 3)
                  strcpy(upgrade_string, "+10% duration");
               else if (random_upgrade[1] == 4)
                  strcpy(upgrade_string, "+10% effectiveness");
               else if (random_upgrade[1] == 5)
                  strcpy(upgrade_string, "+10% speed");
            }
            else
            {
               if (spelllist[random[2]].type == aoe || spelllist[random[2]].type == explosive)
               {
                  random_upgrade[1] = m_rand() % 5; // damage, mana, cooldown, duration, size
                  // explosiondamage, mana, cooldown, explosionduration, explosionsize
               }
               if (random_upgrade[1] == 0)
                  strcpy(upgrade_string, "+10% damage");
               else if (random_upgrade[1] == 1)
                  strcpy(upgrade_string, "-20% cost");
               else if (random_upgrade[1] == 2)
                  strcpy(upgrade_string, "-10% cooldown");
               else if (random_upgrade[1] == 3)
                  strcpy(upgrade_string, "+10% duration");
               else if (random_upgrade[1] == 4)
                  strcpy(upgrade_string, "+30% size");
            }
            m_font_draw(&window, &font_default, upgrade_string, 15, 140, window.height * 5 / 7 + 20, color);
         }
         drawonce = true;
      }
      if (ctoy_mouse_button_press(CTOY_MOUSE_BUTTON_LEFT))
      {
         static short select = -1;
         float mouse_x = ctoy_mouse_x();
         float mouse_y = ctoy_mouse_y();
         if (mouse_x > 0.1 && mouse_x < 0.8)
         {
            if (mouse_y * 7.0F > 1.0F && mouse_y * 7.0F < 2.0F)
            {
               select = 0;
            }
            else if (mouse_y * 7.0F > 3.0F && mouse_y * 7.0F < 4.0F)
            {
               if (player.lvl < 6)
               {
                  select = 1;
               }
               else
               {
                  if (spelllist[random[1]].type == explosive)
                  {
                     if (random_upgrade[0] == 0)
                     {
                        spelllist[random[1]].explosiondamage *= 1.1;
                     }
                     else if (random_upgrade[0] == 3)
                     {
                        spelllist[random[1]].explosionduration *= 1.1;
                     }
                     else if (random_upgrade[0] == 4)
                     {
                        spelllist[random[1]].explosionrange *= 1.3;
                     }
                  }
                  else if (spelllist[random[1]].type == special)
                  {
                     if (random_upgrade[0] == 3)
                     {
                        spelllist[random[1]].range *= 1.1;
                     }
                     else if (random_upgrade[0] == 4)
                     {
                        spelllist[random[1]].explosiondamage *= 1.1;
                     }
                     else if (random_upgrade[0] == 5)
                     {
                        spelllist[random[1]].speed *= 1.1;
                     }
                  }
                  else if (spelllist[random[1]].type == aoe)
                  {
                     if (random_upgrade[0] == 3)
                     {
                        spelllist[random[1]].range *= 1.1;
                     }
                     else if (random_upgrade[0] == 4)
                     {
                        spelllist[random[1]].width *= 1.3;
                     }
                  }
                  if (random_upgrade[0] == 0)
                  {
                     spelllist[random[1]].damage *= 1.1;
                  }
                  else if (random_upgrade[0] == 1)
                  {
                     spelllist[random[1]].mana *= 0.8;
                  }
                  else if (random_upgrade[0] == 2)
                  {
                     spelllist[random[1]].cooldown *= 0.9;
                  }
                  drawonce = false;
                  level_up_screen = false;
               }
            }
            else if (mouse_y * 7.0F > 5.0F && mouse_y * 7.0F < 6.0F)
            {
               if (player.lvl < 6)
               {
                  select = 2;
               }
               else
               {
                  if (spelllist[random[2]].type == explosive)
                  {
                     if (random_upgrade[1] == 0)
                     {
                        spelllist[random[2]].explosiondamage *= 1.1;
                     }
                     else if (random_upgrade[1] == 3)
                     {
                        spelllist[random[2]].explosionduration *= 1.1;
                     }
                     else if (random_upgrade[1] == 4)
                     {
                        spelllist[random[2]].explosionrange *= 1.3;
                     }
                  }
                  else if (spelllist[random[2]].type == special)
                  {
                     if (random_upgrade[1] == 3)
                     {
                        spelllist[random[2]].range *= 1.1;
                     }
                     else if (random_upgrade[1] == 4)
                     {
                        spelllist[random[2]].explosiondamage *= 1.1;
                     }
                     else if (random_upgrade[1] == 5)
                     {
                        spelllist[random[2]].speed *= 1.1;
                     }
                  }
                  else if (spelllist[random[1]].type == aoe)
                  {
                     if (random_upgrade[1] == 3)
                     {
                        spelllist[random[2]].range *= 1.1;
                     }
                     else if (random_upgrade[1] == 4)
                     {
                        spelllist[random[2]].width *= 1.3;
                     }
                  }
                  if (random_upgrade[1] == 0)
                  {
                     spelllist[random[2]].damage *= 1.1;
                  }
                  else if (random_upgrade[1] == 1)
                  {
                     spelllist[random[2]].mana *= 0.8;
                  }
                  else if (random_upgrade[1] == 2)
                  {
                     spelllist[random[2]].cooldown *= 0.9;
                  }
                  drawonce = false;
                  level_up_screen = false;
               }
            }
            else
            {
               select = -1;
            }
            if (spelllist[0].damage / (spelllist[0].mana * spelllist[0].cooldown) > 0.6 && spelllist[0].damage / (spelllist[0].mana * spelllist[0].cooldown) < 0.8) {
               spelllist[0].cooldown = 10;
               spelllist[0].damage = 80;
               spelllist[0].mana = 8; 
               spelllist[0].width = 24;
            }
         }
         else if (select != -1 && mouse_x > 20.00 / window.width && mouse_x < 40.0 / window.width)
         {
            if (mouse_y * window.height > 20 && mouse_y * window.height < 40)
            {
               player.spells[0] = random[select];
               drawonce = false;
               level_up_screen = false;
            }
            else if (mouse_y * window.height > 60 && mouse_y * window.height < 80)
            {
               player.spells[1] = random[select];
               drawonce = false;
               level_up_screen = false;
            }
            else if (mouse_y * window.height > 100 && mouse_y * window.height < 120)
            {
               player.spells[2] = random[select];
               drawonce = false;
               level_up_screen = false;
            }
            else if (mouse_y * window.height > 140 && mouse_y * window.height < 160)
            {
               player.spells[3] = random[select];
               drawonce = false;
               level_up_screen = false;
            }
            else if (mouse_y * window.height > 180 && mouse_y * window.height < 200)
            {
               player.spells[4] = random[select];
               drawonce = false;
               level_up_screen = false;
            }
            select = -1;
         }
      }
   }
   else
   {
      pixel_init();

      m_image_resize(&cooldownbar, &cooldownbar, window.width / 3, window.height / 3);
      float color[] = {0.0, 1.0, 0.0};
      m_font_draw(&cooldownbar, &font_default, "You died!", strlen("You died!"), 0, 0, color);
      m_image_resize(&cooldownbar, &cooldownbar, window.width, window.height);
      m_image_draw(&window, &cooldownbar, 0, 0);
   }
   ctoy_swap_buffer(&window);
}

void ctoy_end(void)
{

   FILE *log = fopen("log.txt", "a");
   fprintf(log, "%s: %f\n", playername, (float)time / 1000 * player.lvl * player.lvl);
   fclose(log);

   m_image_destroy(&window);
   m_image_destroy(&font_default);
   m_image_destroy(&player.image);
   m_image_destroy(&game_objects);
   m_image_destroy(&background);
   m_image_destroy(&background_cutout);
   m_image_destroy(&cooldownbar);
   m_image_destroy(&enemy_cutout);
   m_image_destroy(&proj_cutout);
   m_image_destroy(&small_enemy);

   for (int i = 0; i < 16; i++)
   {
      m_image_destroy(&spelllist[i].projectile_image);
   }
   for (proj_t *temp = projectilelist.next, *prevtemp; temp != NULL;)
   {
      prevtemp = temp;
      temp = temp->next;
      free(prevtemp);
   }

   for (enemy_t *temp = enemylist.next, *prevtemp; temp != NULL;)
   {
      prevtemp = temp;
      temp = temp->next;
      free(prevtemp);
   }
}
