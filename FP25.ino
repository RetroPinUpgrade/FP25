 /**************************************************************************
    This pinball code is distributed in the hope that it
    will be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <https://www.gnu.org/licenses/>.
*/

#include "RPU_Config.h"
#include "RPU.h"
#include "FP.h"
#include "OperatorMenus.h"
#include "AudioHandler.h"
#include "DisplayHandler.h"
#include "LampAnimations.h"
#include <EEPROM.h>

#define GAME_MAJOR_VERSION  2025
#define GAME_MINOR_VERSION  1
#define DEBUG_MESSAGES  1

#if (DEBUG_MESSAGES==1)
//#define DEBUG_SHOW_LOOPS_PER_SECOND
#endif

/*********************************************************************

    Game specific code

*********************************************************************/

// MachineState
//  0 - Attract Mode
//  negative - no longer used
//  positive - game play
boolean InOperatorMenu = false;
char MachineState = 0;
boolean MachineStateChanged = true;
#define MACHINE_STATE_ATTRACT         0
#define MACHINE_STATE_INIT_GAMEPLAY   1
#define MACHINE_STATE_INIT_NEW_BALL   2
#define MACHINE_STATE_NORMAL_GAMEPLAY 4
#define MACHINE_STATE_COUNTDOWN_BONUS 99
#define MACHINE_STATE_BALL_OVER       100
#define MACHINE_STATE_MATCH_MODE      110
#define MACHINE_STATE_DIAGNOSTICS     120

// Indices of EEPROM save locations
#define EEPROM_RPOS_INIT_PROOF_UL                 90

// This value needs to be set to a UNIQUE value for the 
// game code. 
#define RPOS_INIT_PROOF                           0x46503031
#define EEPROM_BALL_SAVE_BYTE                     100
#define EEPROM_FREE_PLAY_BYTE                     101
#define EEPROM_TILT_WARNING_BYTE                  104
#define EEPROM_AWARD_OVERRIDE_BYTE                105
#define EEPROM_BALLS_OVERRIDE_BYTE                106
#define EEPROM_TOURNAMENT_SCORING_BYTE            107
#define EEPROM_SFX_VOLUME_BYTE                    108
#define EEPROM_MUSIC_VOLUME_BYTE                  109
#define EEPROM_SCROLLING_SCORES_BYTE              110
#define EEPROM_CALLOUTS_VOLUME_BYTE               111
#define EEPROM_CRB_HOLD_TIME                      118
#define EEPROM_TROUGH_EJECT_STRENGTH              131
#define EEPROM_SAUCER_EJECT_STRENGTH              134
#define EEPROM_SLINGSHOT_STRENGTH                 135 
#define EEPROM_POP_BUMPER_STRENGTH                136
#define EEPROM_GAME_RULES_SELECTION               137
#define EEPROM_MATCH_FEATURE_BYTE                 138
#define EEPROM_EXTRA_BALL_SCORE_UL                160
#define EEPROM_SPECIAL_SCORE_UL                   164


#define GAME_MODE_SKILL_SHOT                        1
#define GAME_MODE_WAIT_FOR_BALL                     2
#define GAME_MODE_UNSTRUCTURED_PLAY                 3
#define GAME_MODE_START_TRAINING                    10
#define GAME_MODE_TRAINING_SUCCESSFUL               19
#define GAME_MODE_OFFER_BATTLE_1                    20
#define GAME_MODE_START_BATTLE_1                    21
#define GAME_MODE_BATTLE_1                          22
#define GAME_MODE_BATTLE_1_WON                      23
#define GAME_MODE_BATTLE_1_LOST                     24
#define GAME_MODE_OFFER_BATTLE_2                    30
#define GAME_MODE_START_BATTLE_2                    31
#define GAME_MODE_BATTLE_2                          32
#define GAME_MODE_START_BATTLE_3                    41
#define GAME_MODE_BATTLE_3                          42
#define GAME_MODE_BOSS_BATTLE_START                 50
#define GAME_MODE_BOSS_BATTLE_WAIT_FOR_BALLS        51
#define GAME_MODE_BOSS_BATTLE                       52
#define GAME_MODE_BOSS_BATTLE_ENDING                53
#define GAME_MODE_BOSS_BATTLE_WON                   54
#define GAME_MODE_BOSS_BATTLE_LOST                  55


#define SOUND_EFFECT_NONE                     0

#define SOUND_EFFECT_TILT                     3
#define SOUND_EFFECT_TILT_WARNING             4
#define SOUND_EFFECT_SCORE_TICK               5
#define SOUND_EFFECT_POP_BUMPER               6
#define SOUND_EFFECT_LEFT_SLING               7
#define SOUND_EFFECT_RIGHT_SLING              8
#define SOUND_EFFECT_STANDUP_BANK_COMPLETED   9
#define SOUND_EFFECT_ALL_STANDUPS_COMPLETED   10
#define SOUND_EFFECT_STANDUP_HIT              11
#define SOUND_EFFECT_STANDUP_REHIT            12
#define SOUND_EFFECT_FIRE_HIT                 13
#define SOUND_EFFECT_FIRE_REHIT               14
#define SOUND_EFFECT_FIRE_COMPLETED           15

#define SOUND_EFFECT_POWER_HIT_1              17
#define SOUND_EFFECT_POWER_HIT_2              18
#define SOUND_EFFECT_POWER_REHIT              19
#define SOUND_EFFECT_POWER_COMPLETED          20
#define SOUND_EFFECT_GAME_OVER                21
#define SOUND_EFFECT_FIRE_INCREASED           22
#define SOUND_EFFECT_POWER_INCREASED          23
#define SOUND_EFFECT_FIREPOWER_AWARD          24
#define SOUND_EFFECT_MATCH_SPIN               28
#define SOUND_EFFECT_MULTIBALL_PULSE          29
#define SOUND_EFFECT_10PT_REHIT               31
#define SOUND_EFFECT_SPINNER_LIT              42
#define SOUND_EFFECT_SPINNER_UNLIT            43
#define SOUND_EFFECT_BATTLE_SPINNER           44
#define SOUND_EFFECT_INLANE_1                 45
#define SOUND_EFFECT_INLANE_2                 46
#define SOUND_EFFECT_INLANE_3                 47
#define SOUND_EFFECT_OUTLANE_1                48
#define SOUND_EFFECT_COUNTDOWN_1              50
#define SOUND_EFFECT_BATTLE_STANDUP_HIT       52
#define SOUND_EFFECT_BATTLE_BULLSEYE          55
#define SOUND_EFFECT_BULLSEYE                 56
#define SOUND_EFFECT_REFUELING                57


#define SOUND_EFFECT_STARTUP_1                100
#define SOUND_EFFECT_STARTUP_2                101

#define SOUND_EFFECT_COIN_DROP_1              105
#define SOUND_EFFECT_COIN_DROP_2              106
#define SOUND_EFFECT_COIN_DROP_3              107

// Game play status callouts
#define SOUND_EFFECT_VP_LEFT_SHIP                     230
#define SOUND_EFFECT_VP_TOP_SHIP                      231
#define SOUND_EFFECT_VP_RIGHT_SHIP                    232
#define SOUND_EFFECT_VP_READY                         233
#define SOUND_EFFECT_VP_HAS_WEAPONS_LEVEL_1           234
#define SOUND_EFFECT_VP_HAS_WEAPONS_LEVEL_2           235
#define SOUND_EFFECT_VP_HAS_WEAPONS_LEVEL_3           236
#define SOUND_EFFECT_VP_HAS_THRUSTERS_LEVEL_1         237
#define SOUND_EFFECT_VP_HAS_THRUSTERS_LEVEL_2         238
#define SOUND_EFFECT_VP_HAS_THRUSTERS_LEVEL_3         239
#define SOUND_EFFECT_VP_WEAPONS_UPGRADED              240
#define SOUND_EFFECT_VP_THRUSTERS_UPGRADED            241
#define SOUND_EFFECT_VP_WEAPONS_LEVEL_1               244
#define SOUND_EFFECT_VP_WEAPONS_LEVEL_2               245
#define SOUND_EFFECT_VP_WEAPONS_LEVEL_3               246
#define SOUND_EFFECT_VP_THRUSTERS_LEVEL_1             247
#define SOUND_EFFECT_VP_THRUSTERS_LEVEL_2             248
#define SOUND_EFFECT_VP_THRUSTERS_LEVEL_3             249

#define SOUND_EFFECT_VP_RANK_INCREASED                250
#define SOUND_EFFECT_VP_YOUR_RANK_IS                  251
#define SOUND_EFFECT_VP_YOUR_RANK_IS_NOW              252
#define SOUND_EFFECT_VP_YOUR_NEW_RANK_IS              253
#define SOUND_EFFECT_VP_LETS_GO                       254
#define SOUND_EFFECT_VP_GET_OFF_THE_GROUND            255
#define SOUND_EFFECT_VP_BATTLE_AWAITS                 256

#define NICKNAMES_LEVEL_1_START   260
#define NICKNAMES_LEVEL_1_QTY     4
#define NICKNAMES_LEVEL_2_START   272
#define NICKNAMES_LEVEL_2_QTY     3
#define NICKNAMES_LEVEL_3_START   281
#define NICKNAMES_LEVEL_3_QTY     2
#define NICKNAMES_LEVEL_4_START   287
#define NICKNAMES_LEVEL_4_QTY     2
#define NICKNAMES_LEVEL_5_START   293
#define NICKNAMES_LEVEL_5_QTY     1

#define SOUND_EFFECT_VP_NAME_1_1A                       260
#define SOUND_EFFECT_VP_NAME_1_1B                       261
#define SOUND_EFFECT_VP_NAME_1_1C                       262
#define SOUND_EFFECT_VP_NAME_1_2A                       263
#define SOUND_EFFECT_VP_NAME_1_2B                       264
#define SOUND_EFFECT_VP_NAME_1_2C                       265
#define SOUND_EFFECT_VP_NAME_1_3A                       266
#define SOUND_EFFECT_VP_NAME_1_3B                       267
#define SOUND_EFFECT_VP_NAME_1_3C                       268
#define SOUND_EFFECT_VP_NAME_1_4A                       269
#define SOUND_EFFECT_VP_NAME_1_4B                       270
#define SOUND_EFFECT_VP_NAME_1_4C                       271
#define SOUND_EFFECT_VP_NAME_2_1A                       272
#define SOUND_EFFECT_VP_NAME_2_1B                       273
#define SOUND_EFFECT_VP_NAME_2_1C                       274
#define SOUND_EFFECT_VP_NAME_2_2A                       275
#define SOUND_EFFECT_VP_NAME_2_2B                       276
#define SOUND_EFFECT_VP_NAME_2_2C                       277
#define SOUND_EFFECT_VP_NAME_2_3A                       278
#define SOUND_EFFECT_VP_NAME_2_3B                       279
#define SOUND_EFFECT_VP_NAME_2_3C                       280
#define SOUND_EFFECT_VP_NAME_3_1A                       281
#define SOUND_EFFECT_VP_NAME_3_1B                       282
#define SOUND_EFFECT_VP_NAME_3_1C                       283
#define SOUND_EFFECT_VP_NAME_3_2A                       284
#define SOUND_EFFECT_VP_NAME_3_2B                       285
#define SOUND_EFFECT_VP_NAME_3_2C                       286
#define SOUND_EFFECT_VP_NAME_4_1A                       287
#define SOUND_EFFECT_VP_NAME_4_1B                       288
#define SOUND_EFFECT_VP_NAME_4_1C                       289
#define SOUND_EFFECT_VP_NAME_4_2A                       290
#define SOUND_EFFECT_VP_NAME_4_2B                       291
#define SOUND_EFFECT_VP_NAME_4_2C                       292
#define SOUND_EFFECT_VP_NAME_5_1A                       293
#define SOUND_EFFECT_VP_NAME_5_1B                       294
#define SOUND_EFFECT_VP_NAME_5_1C                       295
#define SOUND_EFFECT_VP_TRAINING_WAIT                   296
#define SOUND_EFFECT_VP_TRAINING_GOOD_ENOUGH            297

#define SOUND_EFFECT_VP_BALL_MISSING                    300
#define SOUND_EFFECT_VP_PLAYER_1_UP                     301
#define SOUND_EFFECT_VP_PLAYER_2_UP                     302
#define SOUND_EFFECT_VP_PLAYER_3_UP                     303
#define SOUND_EFFECT_VP_PLAYER_4_UP                     304
#define SOUND_EFFECT_VP_EXTRA_BALL                      305

#define SOUND_EFFECT_VP_ADD_PLAYER_1        306
#define SOUND_EFFECT_VP_ADD_PLAYER_2        (SOUND_EFFECT_VP_ADD_PLAYER_1+1)
#define SOUND_EFFECT_VP_ADD_PLAYER_3        (SOUND_EFFECT_VP_ADD_PLAYER_1+2)
#define SOUND_EFFECT_VP_ADD_PLAYER_4        (SOUND_EFFECT_VP_ADD_PLAYER_1+3)
#define SOUND_EFFECT_VP_SHOOT_AGAIN         310
#define SOUND_EFFECT_VP_BALL_LOCKED                     313
#define SOUND_EFFECT_VP_BALL_SAVE                       326
#define SOUND_EFFECT_VP_SKILL_SHOT_1                    330
#define SOUND_EFFECT_VP_SKILL_SHOT_2                    331
#define SOUND_EFFECT_VP_SKILL_SHOT_3                    332
#define SOUND_EFFECT_VP_SKILL_SHOT_4                    333
#define SOUND_EFFECT_VP_SKILL_SHOT_5                    334
#define SOUND_EFFECT_VP_SKILL_SHOT_6                    335
#define SOUND_EFFECT_VP_SKILL_SHOT_7                    336
#define SOUND_EFFECT_VP_SUPER_SKILL_SHOT_1              337
#define SOUND_EFFECT_VP_SUPER_SKILL_SHOT_2              338
#define SOUND_EFFECT_VP_SUPER_SKILL_SHOT_3              339
#define SOUND_EFFECT_VP_LEFT_DOCK_READY_LONG            340
#define SOUND_EFFECT_VP_LEFT_DOCK_READY                 341
#define SOUND_EFFECT_VP_TOP_DOCK_READY_LONG             342
#define SOUND_EFFECT_VP_TOP_DOCK_READY                  343
#define SOUND_EFFECT_VP_RIGHT_DOCK_READY_LONG           344
#define SOUND_EFFECT_VP_RIGHT_DOCK_READY                345
#define SOUND_EFFECT_VP_EQUIP_THE_SHIP                  346
#define SOUND_EFFECT_VP_ADD_WEAPONS                     347
#define SOUND_EFFECT_VP_ADD_THRUSTERS                   348
#define SOUND_EFFECT_VP_REFUELING                       349

#define SOUND_EFFECT_VP_STARTING_DOCKING_TRAINING       350
#define SOUND_EFFECT_VP_REFRESHER_DOCKING_TRAINING      351
#define SOUND_EFFECT_VP_ADDING_DOCKING_TRAINING         352
#define SOUND_EFFECT_VP_DOCKING_TRAINING_EXPLAINER      353
#define SOUND_EFFECT_VP_DOCKING_TRAINING_HALFWAY        354
#define SOUND_EFFECT_VP_DOCKING_TRAINING_COMPLETE       355
#define SOUND_EFFECT_VP_DOCKING_TRAINING_INCOMPLETE     356
#define SOUND_EFFECT_VP_RESUME_DOCKING_TRAINING         357

#define SOUND_EFFECT_VP_STARTING_WEAPONS_TRAINING       360
#define SOUND_EFFECT_VP_REFRESHER_WEAPONS_TRAINING      361
#define SOUND_EFFECT_VP_ADDING_WEAPONS_TRAINING         362
#define SOUND_EFFECT_VP_WEAPONS_TRAINING_EXPLAINER      363
#define SOUND_EFFECT_VP_WEAPONS_TRAINING_HALFWAY        364
#define SOUND_EFFECT_VP_WEAPONS_TRAINING_COMPLETE       365
#define SOUND_EFFECT_VP_WEAPONS_TRAINING_INCOMPLETE     366
#define SOUND_EFFECT_VP_RESUME_WEAPONS_TRAINING         367
#define SOUND_EFFECT_VP_WEAPONS_TRAINING_QUALIFIED      368

#define SOUND_EFFECT_VP_STARTING_FLIGHT_TRAINING        370
#define SOUND_EFFECT_VP_REFRESHER_FLIGHT_TRAINING       371
#define SOUND_EFFECT_VP_ADDING_FLIGHT_TRAINING          372
#define SOUND_EFFECT_VP_FLIGHT_TRAINING_EXPLAINER       373
#define SOUND_EFFECT_VP_FLIGHT_TRAINING_HALFWAY         374
#define SOUND_EFFECT_VP_FLIGHT_TRAINING_COMPLETE        375
#define SOUND_EFFECT_VP_FLIGHT_TRAINING_INCOMPLETE      376
#define SOUND_EFFECT_VP_RESUME_FLIGHT_TRAINING          377
#define SOUND_EFFECT_VP_FLIGHT_TRAINING_QUALIFIED       378

#define SOUND_EFFECT_VP_STARTING_NAVIGATION_TRAINING    380
#define SOUND_EFFECT_VP_REFRESHER_NAVIGATION_TRAINING   381
#define SOUND_EFFECT_VP_ADDING_NAVIGATION_TRAINING      382
#define SOUND_EFFECT_VP_NAVIGATION_TRAINING_EXPLAINER   383
#define SOUND_EFFECT_VP_NAVIGATION_TRAINING_HALFWAY     384
#define SOUND_EFFECT_VP_NAVIGATION_TRAINING_COMPLETE    385
#define SOUND_EFFECT_VP_NAVIGATION_TRAINING_INCOMPLETE  386
#define SOUND_EFFECT_VP_RESUME_NAVIGATION_TRAINING      387

#define SOUND_EFFECT_VP_JACKPOT_1                       390
#define SOUND_EFFECT_VP_JACKPOT_2                       391
#define SOUND_EFFECT_VP_JACKPOT_3                       392
#define SOUND_EFFECT_VP_JACKPOT_4                       393
#define SOUND_EFFECT_VP_JACKPOT_5                       394
#define SOUND_EFFECT_VP_JACKPOT_6                       395
#define SOUND_EFFECT_VP_DOUBLE_JACKPOT                  396
#define SOUND_EFFECT_VP_TRIPLE_JACKPOT                  397
#define SOUND_EFFECT_VP_SUPER_JACKPOT                   398
#define SOUND_EFFECT_VP_MEGA_JACKPOT                    399

#define SOUND_EFFECT_VP_RANK_1                          400
#define SOUND_EFFECT_VP_RANK_2                          401
#define SOUND_EFFECT_VP_RANK_3                          402
#define SOUND_EFFECT_VP_RANK_4                          403
#define SOUND_EFFECT_VP_RANK_5                          404
#define SOUND_EFFECT_VP_RANK_6                          405
#define SOUND_EFFECT_VP_RANK_7                          406
#define SOUND_EFFECT_VP_RANK_8                          407
#define SOUND_EFFECT_VP_RANK_9                          408
#define SOUND_EFFECT_VP_RANK_10                         409
#define SOUND_EFFECT_VP_RANK_11                         410
#define SOUND_EFFECT_VP_RANK_12                         411

#define SOUND_EFFECT_VP_OFFER_BATTLE_1                  420
#define SOUND_EFFECT_VP_BATTLE_1_INSTRUCTIONS           421
#define SOUND_EFFECT_VP_BATTLE_SPINNER                  422
#define SOUND_EFFECT_VP_BATTLE_STANDUPS                 423
#define SOUND_EFFECT_VP_BATTLE_BULLSEYE                 424
#define SOUND_EFFECT_VP_BATTLE_SAUCER                   425
#define SOUND_EFFECT_VP_BATTLE_1_COMPLETED              426
#define SOUND_EFFECT_VP_BATTLE_1_FAILED                 427
#define SOUND_EFFECT_VP_BATTLE_1_TEN_SECONDS            428
#define SOUND_EFFECT_VP_TEN_SECONDS_SAUCER_REFUEL       429
#define SOUND_EFFECT_VP_BATTLE_1_OR_2_REJECTED_1        430
#define SOUND_EFFECT_VP_BATTLE_1_OR_2_REJECTED_2        431
#define SOUND_EFFECT_VP_BATTLE_1_OR_2_REJECTED_3        432
#define SOUND_EFFECT_VP_BATTLE_3_REJECTED_WEAPONS       433
#define SOUND_EFFECT_VP_BATTLE_3_REJECTED_THRUSTERS     434
#define SOUND_EFFECT_VP_BATTLE_3_REJECTED_UPGRADE       435
#define SOUND_EFFECT_VP_RESTART_MISSION_SAUCER          436
#define SOUND_EFFECT_VP_LAUNCH_SECOND_OR_LOSE_POWER     437
#define SOUND_EFFECT_VP_LAUNCH_THIRD_OR_LOSE_POWER      438
#define SOUND_EFFECT_VP_POWER_SHUTDOWN_COMING           439
#define SOUND_EFFECT_VP_OFFER_BATTLE_2                  440
#define SOUND_EFFECT_VP_BATTLE_2_NEXT_SHIP              441
#define SOUND_EFFECT_VP_TEN_SECONDS_SHUTDOWN            442
#define SOUND_EFFECT_VP_FIVE                            443
#define SOUND_EFFECT_VP_FOUR                            444
#define SOUND_EFFECT_VP_THREE                           445
#define SOUND_EFFECT_VP_TWO                             446
#define SOUND_EFFECT_VP_ONE                             447
#define SOUND_EFFECT_VP_LAUNCH_THE_SHIP                 448
#define SOUND_EFFECT_VP_PLUNGE_TO_LAUNCH_THE_SHIP       449
#define SOUND_EFFECT_VP_OFFER_BATTLE_3                  450
#define SOUND_EFFECT_VP_RIGHT_FLIPPER_RESUME_MISSION    451
#define SOUND_EFFECT_VP_BATTLE_2_SPINNER                452
#define SOUND_EFFECT_VP_BATTLE_2_STANDUPS               453
#define SOUND_EFFECT_VP_BATTLE_2_BULLSEYE               454
#define SOUND_EFFECT_VP_BATTLE_2_SAUCER                 455
#define SOUND_EFFECT_VP_ENEMIES_LEFT_1                  456
#define SOUND_EFFECT_VP_ENEMIES_LEFT_2                  457
#define SOUND_EFFECT_VP_ENEMIES_LEFT_3                  458
#define SOUND_EFFECT_VP_ENEMIES_LEFT_4                  459
#define SOUND_EFFECT_VP_ENEMIES_LEFT_5                  460
#define SOUND_EFFECT_VP_ENEMIES_LEFT_6                  461
#define SOUND_EFFECT_VP_ENEMIES_LEFT_7                  462
#define SOUND_EFFECT_VP_ENEMIES_LEFT_8                  463
#define SOUND_EFFECT_VP_ENEMIES_LEFT_9                  464
#define SOUND_EFFECT_VP_ENEMIES_LEFT_10                 465
#define SOUND_EFFECT_VP_YOU_GOT_ONE                     466
#define SOUND_EFFECT_VP_ENEMY_DISABLED                  467
#define SOUND_EFFECT_VP_GOOD_JOB_ONE_DOWN               468
#define SOUND_EFFECT_VP_YOU_DISABLED_ANOTHER            469
#define SOUND_EFFECT_VP_YOU_DISABLED_ONE                470
#define SOUND_EFFECT_VP_ONE_DOWN                        471
#define SOUND_EFFECT_VP_ANOTHER_ONE_DOWN                472

// These prompts will remind the player to finish training
// they've already started - followed by nickname
#define SOUND_EFFECT_VP_DOCKING_TRAINING_REMINDER_1     500
#define SOUND_EFFECT_VP_WEAPONS_TRAINING_REMINDER_1     501
#define SOUND_EFFECT_VP_FLIGHT_TRAINING_REMINDER_1      502
#define SOUND_EFFECT_VP_NAVIGATION_TRAINING_REMINDER_1  503
#define SOUND_EFFECT_VP_DOCKING_TRAINING_REMINDER_2     504
#define SOUND_EFFECT_VP_WEAPONS_TRAINING_REMINDER_2     505
#define SOUND_EFFECT_VP_FLIGHT_TRAINING_REMINDER_2      506
#define SOUND_EFFECT_VP_NAVIGATION_TRAINING_REMINDER_2  507
#define SOUND_EFFECT_VP_DOCKING_TRAINING_REMINDER_3     508
#define SOUND_EFFECT_VP_WEAPONS_TRAINING_REMINDER_3     509
#define SOUND_EFFECT_VP_FLIGHT_TRAINING_REMINDER_3      510
#define SOUND_EFFECT_VP_NAVIGATION_TRAINING_REMINDER_3  511

// These prompts will guide the player to work towards
// training missions that they haven't completed yet (and
// aren't yet started) - followed by nickname
#define SOUND_EFFECT_VP_DOCKING_TRAINING_NUDGE_1        515
#define SOUND_EFFECT_VP_WEAPONS_TRAINING_NUDGE_1        516
#define SOUND_EFFECT_VP_FLIGHT_TRAINING_NUDGE_1         517
#define SOUND_EFFECT_VP_NAVIGATION_TRAINING_NUDGE_1     518
#define SOUND_EFFECT_VP_DOCKING_TRAINING_NUDGE_2        519
#define SOUND_EFFECT_VP_WEAPONS_TRAINING_NUDGE_2        520
#define SOUND_EFFECT_VP_FLIGHT_TRAINING_NUDGE_2         521
#define SOUND_EFFECT_VP_NAVIGATION_TRAINING_NUDGE_2     522
#define SOUND_EFFECT_VP_DOCKING_TRAINING_NUDGE_3        523
#define SOUND_EFFECT_VP_WEAPONS_TRAINING_NUDGE_3        524
#define SOUND_EFFECT_VP_FLIGHT_TRAINING_NUDGE_3         525
#define SOUND_EFFECT_VP_NAVIGATION_TRAINING_NUDGE_3     526

#define SOUND_EFFECT_BONUS_COUNT_0                    800
#define SOUND_EFFECT_BONUS_COUNT_1                    801
#define SOUND_EFFECT_BONUS_COUNT_2                    802
#define SOUND_EFFECT_BONUS_COUNT_3                    803
#define SOUND_EFFECT_BONUS_COUNT_4                    804
#define SOUND_EFFECT_BONUS_COUNT_5                    805
#define SOUND_EFFECT_BONUS_COUNT_6                    806
#define SOUND_EFFECT_BONUS_COUNT_7                    807
#define SOUND_EFFECT_BONUS_COUNT_8                    808
#define SOUND_EFFECT_BONUS_COUNT_9                    809

#define SOUND_EFFECT_BACKGROUND_SONG_1                900
#define SOUND_EFFECT_BACKGROUND_SONG_2                901
#define SOUND_EFFECT_BACKGROUND_SONG_3                902
#define SOUND_EFFECT_BACKGROUND_SONG_4                903
#define SOUND_EFFECT_BACKGROUND_SONG_5                904
#define SOUND_EFFECT_BACKGROUND_SONG_6                905
#define SOUND_EFFECT_TRAINING_SONG_1                  910
#define SOUND_EFFECT_TRAINING_SONG_2                  911
#define SOUND_EFFECT_BATTLE_SONG_1                    920
#define SOUND_EFFECT_BATTLE_SONG_2                    921
#define SOUND_EFFECT_BATTLE_SONG_3                    922
#define SOUND_EFFECT_BATTLE_SONG_4                    923
#define SOUND_EFFECT_BATTLE_SONG_5                    924
#define SOUND_EFFECT_BATTLE_SONG_6                    925
#define SOUND_EFFECT_COOLDOWN_SONG_1                  930
#define SOUND_EFFECT_COOLDOWN_SONG_2                  931
#define SOUND_EFFECT_READY_FOR_BATTLE_1               940
#define SOUND_EFFECT_READY_FOR_BATTLE_2               941

#define SOUND_EFFECT_DIAG_START                   1900
#define SOUND_EFFECT_DIAG_CREDIT_RESET_BUTTON     1900
#define SOUND_EFFECT_DIAG_SELECTOR_SWITCH_ON      1901
#define SOUND_EFFECT_DIAG_SELECTOR_SWITCH_OFF     1902
#define SOUND_EFFECT_DIAG_STARTING_ORIGINAL_CODE  1903
#define SOUND_EFFECT_DIAG_STARTING_NEW_CODE       1904
#define SOUND_EFFECT_DIAG_ORIGINAL_CPU_DETECTED   1905
#define SOUND_EFFECT_DIAG_ORIGINAL_CPU_RUNNING    1906
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_U10         1907
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_U11         1908
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_1           1909
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_2           1910
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_3           1911
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_4           1912
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_5           1913
#define SOUND_EFFECT_DIAG_STARTING_DIAGNOSTICS    1914


#define MAX_DISPLAY_BONUS     39
#define TILT_WARNING_DEBOUNCE_TIME      1000

#define BALL_SAVE_GRACE_PERIOD            3000

/*********************************************************************

    Machine state and options

*********************************************************************/
byte Credits = 0;
byte BallSaveNumSeconds = 0;
byte MaximumCredits = 40;
byte BallsPerGame = 3;
byte ScoreAwardReplay = 0;
byte MusicVolume = 6;
byte SoundEffectsVolume = 8;
byte CalloutsVolume = 10;
byte ChuteCoinsInProgress[3];
byte TotalBallsLoaded = 3;
byte TimeRequiredToResetGame = 1;
boolean FreePlayMode = false;
boolean MatchEnabled = true;
boolean HighScoreReplay = true;
boolean MatchFeature = true;
boolean TournamentScoring = false;
boolean ScrollingScores = true;
unsigned long ExtraBallValue = 0;
unsigned long SpecialValue = 0;
unsigned long CurrentTime = 0;
unsigned long HighScore = 0;
unsigned long AwardScores[3];
unsigned long CreditResetPressStarted = 0;
unsigned long OperatorSwitchPressStarted = 0;

#define NUM_CPC_PAIRS 9
boolean CPCSelectionsHaveBeenRead = false;
byte CPCPairs[NUM_CPC_PAIRS][2] = {
  {1, 5},
  {1, 4},
  {1, 3},
  {1, 2},
  {1, 1},
  {2, 3},
  {2, 1},
  {3, 1},
  {4, 1}
};
byte CPCSelection[3];

AudioHandler  Audio;
OperatorMenus Menus;



/*********************************************************************

    Game State

*********************************************************************/
byte CurrentPlayer = 0;
byte CurrentBallInPlay = 1;
byte CurrentNumPlayers = 0;
byte NumberOfBallsLocked;
byte NumberOfBallsInPlay;
byte Bonus[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte BonusX[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte GameMode = GAME_MODE_SKILL_SHOT;
byte LastGameMode = 0;
byte MaxTiltWarnings = 2;
byte NumTiltWarnings = 0;
byte CurrentAchievements[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte NumberOfBallSavesRemaining;
byte SaucerSwitches[3] = {SW_LEFT_SAUCER, SW_TOP_SAUCER, SW_RIGHT_SAUCER};
byte SaucerSolenoids[3] = {SOL_LEFT_SAUCER, SOL_TOP_SAUCER, SOL_RIGHT_SAUCER};
byte MachineLocks;
byte PlayerLocks[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte LeftKickback[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte GameModeStage;
byte ResetGameSong = 0;
byte LeftInlaneStage;
byte RightInlaneStage;
// These are in milliseconds/8
byte FirePowerIncreaseLampOffsets[] = {0, 45, 77, 115, 136, 163, 184, 211, 227, 248};

#define BALL_LEFT_LOCK_QUALIFIED    0x01
#define BALL_TOP_LOCK_QUALIFIED     0x02
#define BALL_RIGHT_LOCK_QUALIFIED   0x04
#define BALL_LOCKS_QUALIFIED        0x07
#define BALL_LEFT_LOCK_ENGAGED      0x10
#define BALL_TOP_LOCK_ENGAGED       0x20
#define BALL_RIGHT_LOCK_ENGAGED     0x40
#define BALL_LOCKS_ENGAGED          0x70

boolean SamePlayerShootsAgain = false;
boolean BallSaveUsed = false;
boolean SpecialAvailable = false;
boolean SpecialCollected = false;
boolean TimersPaused = false;
boolean DisplaysNeedRefreshing = false;
boolean RightFlipperHeld;
boolean RefuelingMessagePlayed = false;
boolean PowerComboCollected;

unsigned long CurrentScores[RPU_NUMBER_OF_PLAYERS_ALLOWED];
unsigned long BallFirstSwitchHitTime = 0;
unsigned long BallTimeInTrough = 0;
unsigned long GameModeStartTime = 0;
unsigned long GameModeEndTime = 0;
unsigned long LastTiltWarningTime;
unsigned long PlayfieldMultiplier;
unsigned long LastTimeThroughLoop;
unsigned long LastSwitchHitTime;
unsigned long BallSaveEndTime;
unsigned long BallRampKicked;
unsigned long LastSaucerEjectTime[3];
unsigned long LastTimePromptPlayed = 0;
unsigned long LastTimeAwardsChecked = 0;
unsigned long LastTimeJackpotAdjusted = 0;
unsigned long LastLoopTick = 0;
unsigned long LeftInlaneLastHitTime;
unsigned long RightInlaneLastHitTime;
unsigned long LastCoinSwitchTime[3];
unsigned long LastStartButtonSwitchTime;

#define BATTLE_ONE_FINISH_EJECT       0
#define BATTLE_ONE_FINISH_REQUALIFY   1
#define BATTLE_ONE_FINISH_OFFER_NEXT  2

/*********************************************************************

    Game Specific State Variables

*********************************************************************/
byte ExtraBallAvailable[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte BallKickerBehavior;
byte GameRulesSelection;
byte BallServeSolenoidStrength = 50;
byte SaucerSolenoidStrength = 50;
byte TrainingDuration = 30;
byte TrainingRestartTime = 15;
byte PlayerTrainingStatus[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte PlayerNickname[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte NumberOfSpinnerHitsForFlightTraining = 100;
byte NumberOfPopHitsForWeaponsTraining = 50;
byte ComboDuration = 3; // in seconds
byte BattleFuelTime = 60; // in seconds
byte NumberOfBattlesBeforeWizard = 6;
byte BattleOneFinishBehavior = BATTLE_ONE_FINISH_OFFER_NEXT;
byte BallSaveNewBall = 0;
byte BallSaveAfterLock = 0;

#define WEAPONS_TRAINING                  0x01
#define FLIGHT_TRAINING                   0x02

#define WEAPONS_TRAINING_QUALIFIED        0x01
#define FLIGHT_TRAINING_QUALIFIED         0x02
#define TRAINING_QUALIFIED_MASK           0x03

#define WEAPONS_TRAINING_RUNNING          0x04
#define FLIGHT_TRAINING_RUNNING           0x08
#define TRAINING_RUNNING_MASK             0x0C

#define WEAPONS_TRAINING_ACHIEVED_MASK    0x30
#define WEAPONS_TRAINING_ACHIEVED_SHIFT   16
#define FLIGHT_TRAINING_ACHIEVED_MASK     0xC0
#define FLIGHT_TRAINING_ACHIEVED_SHIFT    64

#define BALL_KICKER_EASY                  1
#define BALL_KICKER_MEDIUM                2
#define BALL_KICKER_HARD                  3
#define BALL_KICKER_VERY_HARD             4
#define BALL_KICKER_OFF                   5

byte SkillShotType[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte SkillShotsHit[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte SkillShotLane;
byte TrainingBonus = 50; // in thousands
byte OfferBattleSaucer;
byte SecondBattleSaucer;
byte ThirdBattleSaucer;
byte BattleStage;
byte BattleStageShots;
byte BossBattleStage;
byte BossBattleStagesDone;
byte BossBattleShotThreshold[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte BattlesWon[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte BattleStandups;
byte LastBattleStandupHit;
byte WeaponsTrainingStartingHits;
byte FlightTrainingStartingHits;
byte BossBattlePlungeGrace;

#define BATTLE_STAGE_OFF            0
#define BATTLE_STAGE_SPINNER        1
#define BATTLE_STAGE_STANDUPS       2
#define BATTLE_STAGE_BULLSEYE       3
#define BATTLE_STAGE_SAUCER         4
#define BATTLE_STAGE_WON            5
#define BOSS_STAGE_FIRE_LANES       1
#define BOSS_STAGE_POP_BUMPERS      2
#define BOSS_STAGE_NUMBER_STANDUPS  3
#define BOSS_STAGE_POWER_STANDUPS   4
#define BOSS_STAGE_TOTAL_STAGES     4
#define BOSS_BATTLE_ALL_STAGES      0x0F
byte BossHitsInStage[BOSS_STAGE_TOTAL_STAGES];

byte StandupTargetStatus[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte StandupTargetCompletions[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte FireStatus[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte FireCompletions[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte PowerStatus[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte PowerCompletions[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte FirePowerLevel[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte ShipWeapons[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte ShipThrusters[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte PlayerRank[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte PopHits[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte WeaponsTrainingHits[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte SpinnerHits[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte FlightTrainingHits[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte BattlesPlayed[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte PopExtraValueInThousands[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte SpinnerExtraValueInHundreds[RPU_NUMBER_OF_PLAYERS_ALLOWED];


#define SKILL_SHOT_TYPE_ALWAYS_CHANGEABLE_LANE      0
#define SKILL_SHOT_TYPE_SHOOTER_CHANGEABLE_LANE     1
#define SKILL_SHOT_TYPE_AUTO_CHANGING_LANE          2
#define SKILL_SHOT_TYPE_FIXED_LANE                  3
#define SKILL_SHOT_NUMBER_OF_TYPES                  4
#define SKILL_SHOT_REWARD                           50000

boolean SkillShotChangedAfterLaunch;
boolean JackpotReady;
// game play options (serialized)
boolean SkillShotChangesWhenHit = false;
boolean SkillShotQualifiesLock = true;
boolean HoldoverTraining = true;
boolean WeaponsTrainingHolds = false;
boolean FlightTrainingHolds = false;
boolean ResetShipWeaponsEachBall = false;
boolean ResetShipWeaponsEachBattle = false;
boolean ShowStatusInCreditsWindow = true;
boolean SingleBankQualifiesFirstTwoLocks = true;
boolean ComboCompletesPower = true;
boolean LockRequiredForShipUpgrades = false;
boolean BossBattleFuelBurn;


#define GAME_RULES_EASY         1
#define GAME_RULES_MEDIUM       2
#define GAME_RULES_HARD         3
#define GAME_RULES_PROGRESSIVE  4
#define GAME_RULES_CUSTOM       5

unsigned long LastSkillShotChangedTime;
unsigned long PlayfieldMultiplierExpirationTime;
unsigned long BonusChanged;
unsigned long BonusXAnimationStart;
unsigned long LastTimePopHit[4];
unsigned long FireLaneHitTime[4];
unsigned long LastTimeBallServed;
unsigned long LastTimeSaucerSeen[3];
unsigned long LastTimeStandupSeen[6];
unsigned long BossSaucerKickoutTime[3];
unsigned long SaucerClosedStart[3] = {0, 0, 0};
unsigned long FireCompletedTime;
unsigned long PowerCompletedTime;
unsigned long StandupTargetFinishTime;
unsigned long FireLevelChangedTime;
unsigned long PowerLevelChangedTime;
unsigned long FirePowerLevelChangedTime;
unsigned long PowerTargetHitTime[3];
unsigned long WeaponsTrainingEndTime;
unsigned long FlightTrainingEndTime;
unsigned long BallKickerEndTime;
unsigned long LastTimeBattleShotHit;
unsigned long LastTimeTrainingCalloutMade;
unsigned long LastTimeWeaponsTrainingHit;
unsigned long LastTimeFlightTrainingHit;
unsigned long LastTimeLockShotHit;
unsigned long LastTrainingHitTime;
unsigned long LastTimeShipUpgraded;
unsigned long RightFlipperDown;
unsigned long BattleStandupsLastChange;
unsigned long LastBattleStandupHitTime;
unsigned long BattleRefuelingTime;
unsigned long BossBattleTimer;
unsigned long BossBattleBallInShooter;
unsigned long SwitchHitsInMode;
unsigned long BossBattleBonus;

#define BOSS_BATTLE_SAUCER_HOLD_TIME  8000
#define BOSS_BATTLE_TOTAL_TIME        120000
#define BOSS_BATTLE_LOST_SHIP_PENALTY 5000
#define DROP_TARGET_RESET_STRENGTH    10
#define KNOCKER_SOLENOID_STRENGTH     5
#define POP_ID_BOTTOM_LEFT            0
#define POP_ID_TOP_LEFT               1
#define POP_ID_TOP_RIGHT              2
#define POP_ID_BOTTOM_RIGHT           3

unsigned short JackpotValue[RPU_NUMBER_OF_PLAYERS_ALLOWED];

byte PopBumperLamps[4] = {LAMP_BOTTOM_LEFT_POP, LAMP_TOP_LEFT_POP, LAMP_TOP_RIGHT_POP, LAMP_BOTTOM_RIGHT_POP};
byte PlayerUpLamps[4] = {LAMP_HEAD_PLAYER_1_UP, LAMP_HEAD_PLAYER_2_UP, LAMP_HEAD_PLAYER_3_UP, LAMP_HEAD_PLAYER_4_UP};




/******************************************************

   Adjustments Serialization

*/


void SetAllParameterDefaults() {

  // In the event that the EEPROM has not been initialized,
  // these are the values that will be used
  HighScore = 10000;
  Credits = 4;
  FreePlayMode = false;
  BallSaveNumSeconds = 15;
  BallSaveNewBall = 15;
  BallSaveAfterLock = 5;
  MusicVolume = 10;
  SoundEffectsVolume = 10;
  CalloutsVolume = 10;
  AwardScores[0] = 1000000;
  AwardScores[1] = 3000000;
  AwardScores[2] = 5000000;
  TournamentScoring = false;
  MaxTiltWarnings = 2;
  ScoreAwardReplay = 0x07;
  BallsPerGame = 3;
  ScrollingScores = true;
  MatchFeature = true;
  ExtraBallValue = 20000;
  SpecialValue = 40000;
  TimeRequiredToResetGame = 2;
  CPCSelection[0] = 4;
  CPCSelection[1] = 4;
  CPCSelection[2] = 4;

  // EASY / MEDIUM / HARD rules
  GameRulesSelection = GAME_RULES_MEDIUM;

  SkillShotChangesWhenHit = false;
  SkillShotQualifiesLock = true;
  HoldoverTraining = false;
  WeaponsTrainingHolds = false;
  FlightTrainingHolds = false;
  ResetShipWeaponsEachBall = false;
  ResetShipWeaponsEachBattle = false;
  SingleBankQualifiesFirstTwoLocks = true;
  ShowStatusInCreditsWindow = true;

  TrainingBonus = 50; // in thousands
  TrainingDuration = 30;
  TrainingRestartTime = 15;
  NumberOfSpinnerHitsForFlightTraining = 100;
  NumberOfPopHitsForWeaponsTraining = 50;
  BattleFuelTime = 60; // in seconds
  ComboDuration = 3; // in seconds
  ComboCompletesPower = true;
  LockRequiredForShipUpgrades = false;
  NumberOfBattlesBeforeWizard = 6;
  BattleOneFinishBehavior = BATTLE_ONE_FINISH_OFFER_NEXT;
}


boolean LoadRuleDefaults(byte ruleLevel) {
  // This function just puts the rules in RAM.
  // It's up to the caller to ensure that they're 
  // written to EEPROM when desired (with WriteParameters)
  if (ruleLevel==GAME_RULES_EASY) {
    BallSaveNumSeconds = 20;
    BallSaveNewBall = 20;
    BallSaveAfterLock = 10;
    // Game rules variables initialized to EASY here:
  } else if (ruleLevel==GAME_RULES_MEDIUM) {
    BallSaveNewBall = 15;
    BallSaveAfterLock = 5;
    BallSaveNumSeconds = 15;
    // Game rules variables initialized to MEDIUM here:
  } else if (ruleLevel==GAME_RULES_HARD) {
    BallSaveNewBall = 0;
    BallSaveAfterLock = 0;
    BallSaveNumSeconds = 0;
    // Game rules variables initialized to HARD here:
  } else {
    return false;
  }

  return true;
}


void WriteParameters(boolean onlyWriteRulesParameters = true) {
  if (!onlyWriteRulesParameters) {
    RPU_WriteULToEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, HighScore);
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    RPU_WriteByteToEEProm(RPU_CPC_CHUTE_1_SELECTION_BYTE, CPCSelection[0]);
    RPU_WriteByteToEEProm(RPU_CPC_CHUTE_2_SELECTION_BYTE, CPCSelection[1]);
    RPU_WriteByteToEEProm(RPU_CPC_CHUTE_3_SELECTION_BYTE, CPCSelection[2]);
        
    RPU_WriteByteToEEProm(EEPROM_FREE_PLAY_BYTE, FreePlayMode);
    RPU_WriteByteToEEProm(EEPROM_MUSIC_VOLUME_BYTE, MusicVolume);
    RPU_WriteByteToEEProm(EEPROM_SFX_VOLUME_BYTE, SoundEffectsVolume);
    RPU_WriteByteToEEProm(EEPROM_CALLOUTS_VOLUME_BYTE, CalloutsVolume);

    RPU_WriteULToEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE, AwardScores[0]);
    RPU_WriteULToEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE, AwardScores[1]);
    RPU_WriteULToEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE, AwardScores[2]);

    RPU_WriteByteToEEProm(EEPROM_TOURNAMENT_SCORING_BYTE, TournamentScoring);
    RPU_WriteByteToEEProm(EEPROM_AWARD_OVERRIDE_BYTE, ScoreAwardReplay);
    RPU_WriteByteToEEProm(EEPROM_BALLS_OVERRIDE_BYTE, BallsPerGame);
    RPU_WriteByteToEEProm(EEPROM_SCROLLING_SCORES_BYTE, ScrollingScores);
    RPU_WriteByteToEEProm(EEPROM_MATCH_FEATURE_BYTE, MatchFeature);
    
    RPU_WriteULToEEProm(EEPROM_EXTRA_BALL_SCORE_UL, ExtraBallValue);
    RPU_WriteULToEEProm(EEPROM_SPECIAL_SCORE_UL, SpecialValue);
    RPU_WriteByteToEEProm(EEPROM_CRB_HOLD_TIME, TimeRequiredToResetGame);

    // Set baseline for audits
    RPU_WriteByteToEEProm(RPU_CHUTE_1_COINS_START_BYTE, 0);
    RPU_WriteByteToEEProm(RPU_CHUTE_2_COINS_START_BYTE, 0);
    RPU_WriteByteToEEProm(RPU_CHUTE_3_COINS_START_BYTE, 0);
    RPU_WriteULToEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE, 0);
    RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, 0);
    RPU_WriteULToEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE, 0);
  }

  RPU_WriteByteToEEProm(EEPROM_BALL_SAVE_BYTE, BallSaveNumSeconds);
//    BallSaveNewBall = 15;
//    BallSaveAfterLock = 5;
  RPU_WriteByteToEEProm(EEPROM_TILT_WARNING_BYTE, MaxTiltWarnings); 
  
}

void ReadStoredParameters() {
  for (byte count = 0; count < 3; count++) {
    ChuteCoinsInProgress[count] = 0;
  }

  // The first time the EEPROM has been written with good values for this game,
  // the EEPROM_RPOS_INIT_PROOF_UL will be written to a known state (RPOS_INIT_PROOF)
  // if that value hasn't been written, then we load defaults and save them to EEPROM.
  // This should only happen the first time a device is run with this game code.
  unsigned long RPUProofValue = RPU_ReadULFromEEProm(EEPROM_RPOS_INIT_PROOF_UL, 0);
  if (RPUProofValue!=RPOS_INIT_PROOF) {
    // Doesn't look like this memory has been initialized
    RPU_WriteULToEEProm(EEPROM_RPOS_INIT_PROOF_UL, RPOS_INIT_PROOF);
    SetAllParameterDefaults();
    WriteParameters(false);
  } else {

    // Read machine settings
    HighScore = RPU_ReadULFromEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, 10000);
    Credits = RPU_ReadByteFromEEProm(RPU_CREDITS_EEPROM_BYTE);
    if (Credits > MaximumCredits) Credits = MaximumCredits;
  
    FreePlayMode = ReadSetting(EEPROM_FREE_PLAY_BYTE, false, true);
    MusicVolume = ReadSetting(EEPROM_MUSIC_VOLUME_BYTE, 10, 10);
    SoundEffectsVolume = ReadSetting(EEPROM_SFX_VOLUME_BYTE, 10, 10);
    CalloutsVolume = ReadSetting(EEPROM_CALLOUTS_VOLUME_BYTE, 10, 10);
    Audio.SetMusicVolume(MusicVolume);
    Audio.SetSoundFXVolume(SoundEffectsVolume);
    Audio.SetNotificationsVolume(CalloutsVolume);

    AwardScores[0] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE);
    AwardScores[1] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE);
    AwardScores[2] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE);
  
    TournamentScoring = ReadSetting(EEPROM_TOURNAMENT_SCORING_BYTE, false, true);
    ScoreAwardReplay = ReadSetting(EEPROM_AWARD_OVERRIDE_BYTE, 0x03, 0x07);
    BallsPerGame = ReadSetting(EEPROM_BALLS_OVERRIDE_BYTE, 3, 10);
    ScrollingScores = ReadSetting(EEPROM_SCROLLING_SCORES_BYTE, true, true);
    MatchFeature = ReadSetting(EEPROM_MATCH_FEATURE_BYTE, true, true);

    CPCSelection[0] = ReadSetting(RPU_CPC_CHUTE_1_SELECTION_BYTE, 4, 8);
    CPCSelection[1] = ReadSetting(RPU_CPC_CHUTE_2_SELECTION_BYTE, 4, 8);
    CPCSelection[2] = ReadSetting(RPU_CPC_CHUTE_3_SELECTION_BYTE, 4, 8);
    CPCSelectionsHaveBeenRead = true;

    ExtraBallValue = RPU_ReadULFromEEProm(EEPROM_EXTRA_BALL_SCORE_UL);
    if (ExtraBallValue % 1000 || ExtraBallValue > 1000000) ExtraBallValue = 20000;
  
    SpecialValue = RPU_ReadULFromEEProm(EEPROM_SPECIAL_SCORE_UL);
    if (SpecialValue % 1000 || SpecialValue > 1000000) SpecialValue = 40000;
  
    TimeRequiredToResetGame = ReadSetting(EEPROM_CRB_HOLD_TIME, 1, 99);
    if (TimeRequiredToResetGame > 3 && TimeRequiredToResetGame != 99) TimeRequiredToResetGame = 1;
    
    // Read game rules
    GameRulesSelection = ReadSetting(EEPROM_GAME_RULES_SELECTION, GAME_RULES_MEDIUM, GAME_RULES_CUSTOM);

    BallSaveNumSeconds = ReadSetting(EEPROM_BALL_SAVE_BYTE, 15, 20);
    BallSaveNewBall = 15;
    BallSaveAfterLock = 5;

    MaxTiltWarnings = ReadSetting(EEPROM_TILT_WARNING_BYTE, 2, 3);

    BallKickerBehavior = BALL_KICKER_MEDIUM;
  }
}



byte GetCPCSelection(byte chuteNumber) {
  if (chuteNumber>2) return 0xFF;

  if (CPCSelectionsHaveBeenRead==false) {
    CPCSelection[0] = RPU_ReadByteFromEEProm(RPU_CPC_CHUTE_1_SELECTION_BYTE);
    if (CPCSelection[0]>=NUM_CPC_PAIRS) {
      CPCSelection[0] = 4;
      RPU_WriteByteToEEProm(RPU_CPC_CHUTE_1_SELECTION_BYTE, 4);
    }
    CPCSelection[1] = RPU_ReadByteFromEEProm(RPU_CPC_CHUTE_2_SELECTION_BYTE);
    if (CPCSelection[1]>=NUM_CPC_PAIRS) {
      CPCSelection[1] = 4;
      RPU_WriteByteToEEProm(RPU_CPC_CHUTE_2_SELECTION_BYTE, 4);
    }
    CPCSelection[2] = RPU_ReadByteFromEEProm(RPU_CPC_CHUTE_3_SELECTION_BYTE);
    if (CPCSelection[2]>=NUM_CPC_PAIRS) {
      CPCSelection[2] = 4;
      RPU_WriteByteToEEProm(RPU_CPC_CHUTE_3_SELECTION_BYTE, 4);
    }
    CPCSelectionsHaveBeenRead = true;
  }
  
  return CPCSelection[chuteNumber];
}


byte GetCPCCoins(byte cpcSelection) {
  if (cpcSelection>=NUM_CPC_PAIRS) return 1;
  return CPCPairs[cpcSelection][0];
}


byte GetCPCCredits(byte cpcSelection) {
  if (cpcSelection>=NUM_CPC_PAIRS) return 1;
  return CPCPairs[cpcSelection][1];
}


void QueueDIAGNotification(unsigned short notificationNum) {
  // This is optional, but the machine can play an audio message at boot
  // time to indicate any errors and whether it's going to boot to original
  // or new code.
  //Audio.QueuePrioritizedNotification(notificationNum, 0, 10, CurrentTime);
  (void)notificationNum;
}


// I'm doing this as a function instead of an array because 
// memory is short and spending 44 bytes on a converstion array
// seems wasteful when the board has tons and tons of code space.
// There's a way to store this data in code space and then convert
// it when needed, but that's slow compared to this (ugly) method.
byte LampConvertDisplayNumberToIndex(byte displayNumber) {
  if (displayNumber==0) return OPERATOR_MENU_VALUE_UNUSED;
  if (displayNumber>64) return OPERATOR_MENU_VALUE_OUT_OF_RANGE;

  return displayNumber-1;
}

/*
#define SOL_KNOCKER                 13
#define SOL_OUTHOLE                 7
#define SOL_SERVE_BALL              0
#define SOL_LEFT_SAUCER             3
#define SOL_RIGHT_SAUCER            4
#define SOL_TOP_SAUCER              5
#define SOL_BALLSAVE_KICKER         6
#define SOLCONT_COIN_LOCKOUT        15
*/

unsigned short SolenoidConvertDisplayNumberToIndex(byte displayNumber) {
  switch (displayNumber) {
    case  0: return OPERATOR_MENU_VALUE_UNUSED;
    case  1: return SOL_SERVE_BALL;
    case  2: return OPERATOR_MENU_VALUE_UNUSED;
    case  3: return OPERATOR_MENU_VALUE_UNUSED;
    case  4: return SOL_LEFT_SAUCER;
    case  5: return SOL_RIGHT_SAUCER;
    case  6: return SOL_TOP_SAUCER;
    case  7: return SOL_BALLSAVE_KICKER;
    case  8: return SOL_OUTHOLE;
    case  9: return 8; // sounds
    case 10: return 9;
    case 11: return 10;
    case 12: return 11;
    case 13: return 12;
/*    
    case  9: return OPERATOR_MENU_VALUE_UNUSED;
    case 10: return OPERATOR_MENU_VALUE_UNUSED;
    case 11: return OPERATOR_MENU_VALUE_UNUSED;
    case 12: return OPERATOR_MENU_VALUE_UNUSED;
    case 13: return OPERATOR_MENU_VALUE_UNUSED;
*/    
    case 14: return SOL_KNOCKER;
    case 15: return OPERATOR_MENU_VALUE_UNUSED;
//    case 15: return OPERATOR_MENU_VALUE_UNUSED; 
    case 16: return SOLCONT_COIN_LOCKOUT;
    case 17: return 16; // top left pop
    case 18: return 17; // bottom left pop
    case 19: return 18; // top right pop
    case 20: return 19; // bottom right pop
    case 21: return 20; // right sling
    case 22: return 21; // left sling
    default: return OPERATOR_MENU_VALUE_OUT_OF_RANGE;
  }
}


byte SolenoidConvertDisplayNumberToTestStrength(byte displayNumber) {
  switch (displayNumber) {
    case  0: return OPERATOR_MENU_VALUE_UNUSED;
    case  1: return 4;
    case  2: return OPERATOR_MENU_VALUE_UNUSED;
    case  3: return OPERATOR_MENU_VALUE_UNUSED;
    case  4: return 4;
    case  5: return 4;
    case  6: return 4;
    case  7: return 4;
    case  8: return 4;
    case  9: return OPERATOR_MENU_VALUE_UNUSED;
    case 10: return OPERATOR_MENU_VALUE_UNUSED;
    case 11: return OPERATOR_MENU_VALUE_UNUSED;
    case 12: return OPERATOR_MENU_VALUE_UNUSED;
    case 13: return OPERATOR_MENU_VALUE_UNUSED;
    case 14: return 4;
    case 15: return OPERATOR_MENU_VALUE_UNUSED;
    case 16: return 4;
    case 17: return OPERATOR_MENU_VALUE_UNUSED;
    case 18: return OPERATOR_MENU_VALUE_UNUSED;
    case 19: return OPERATOR_MENU_VALUE_UNUSED; 
    case 20: return OPERATOR_MENU_VALUE_UNUSED; 
    case 21: return OPERATOR_MENU_VALUE_UNUSED; 
    case 22: return OPERATOR_MENU_VALUE_UNUSED; 
    default: return OPERATOR_MENU_VALUE_OUT_OF_RANGE;
  }
}


byte FPSoundTestCallback(byte testNum) {

  if (DEBUG_MESSAGES) {
    Serial.write("Sound test callback ");
    Serial.print(testNum);
    Serial.println("");
  }

  if (testNum==0) {
    Audio.StopAllAudio();
    RPU_SetDisplayBlank(0, 0x00);
    PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_1);
  } else if (testNum==1) {
    Audio.StopSound(SOUND_EFFECT_SPINNER_UNLIT);
    PlaySoundEffect(SOUND_EFFECT_SPINNER_UNLIT);
  } else if (testNum==2) {
    Audio.QueuePrioritizedNotification(SOUND_EFFECT_VP_SKILL_SHOT_1, 0, 10, CurrentTime);
  } else {
    if (testNum==3) Audio.StopAllAudio();
    RPU_SetDisplay(0, testNum-3);
    RPU_SetDisplayBlank(0, 0xFF);
    Audio.PlaySoundCardWhenPossible(19 * 256, CurrentTime, 0, 50, 10);
    Audio.PlaySoundCardWhenPossible((testNum-3) * 256, CurrentTime+150, 0, 2000, 10);
  }

  if (testNum>33) return 0;
  else return 1;
}

void MoveBallFromOutholeToRamp(boolean sawSwitch = false) {
  if (RPU_ReadSingleSwitchState(SW_OUTHOLE) || sawSwitch) {
    if (CurrentTime == 0 || CurrentTime > (BallRampKicked + 1000)) {
      RPU_PushToSolenoidStack(SOL_OUTHOLE, 16, true);
      if (CurrentTime) BallRampKicked = CurrentTime;
      else BallRampKicked = millis();
    }
  }

}


void EjectAllBallsFromSaucers() {
  for (byte count=0; count<3; count++) {
    if (LastSaucerEjectTime[count]==0 || CurrentTime > (LastSaucerEjectTime[count]+2000)) {
      if (RPU_ReadSingleSwitchState(SaucerSwitches[count])) {
//        if (DEBUG_MESSAGES) Serial.write("Ball in saucer\n");
        RPU_PushToSolenoidStack(SaucerSolenoids[count], SaucerSolenoidStrength, true);
        LastSaucerEjectTime[count] = CurrentTime;
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////
//
//  Setup
//    Arduino calls this function at power up and reset.
//    It's used to initialize the hardware and 
//    certain variables and structures used by 
//    the code.
//
////////////////////////////////////////////////////////////////////////////

void setup() {

  if (DEBUG_MESSAGES) {
    // If debug is on, set up the Serial port for communication
    Serial.begin(115200);
    Serial.write("Starting\n");
  }

  // Set up the Audio handler in order to play boot messages
  CurrentTime = millis();
  Audio.InitDevices(AUDIO_PLAY_TYPE_WAV_TRIGGER | AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  Audio.StopAllAudio();
  Audio.SetMusicDuckingGain(25);
  Audio.SetSoundFXDuckingGain(20);

  // Tell the OS about game-specific switches
  // (this is for software-controlled pop bumpers and slings)
#if (RPU_MPU_ARCHITECTURE<10)
  // Machines with a -17, -35, 100, and 200 architecture
  // almost always have software based switch-triggered solenoids.
  // For those, you can define an array of solenoids and the switches
  // that will trigger them:
  RPU_SetupGameSwitches(NUM_SWITCHES_WITH_TRIGGERS, NUM_PRIORITY_SWITCHES_WITH_TRIGGERS, SolenoidAssociatedSwitches);

#endif

  // Set up the chips and interrupts
  unsigned long initResult = 0;
  if (DEBUG_MESSAGES) Serial.write("Initializing MPU\n");

  // If the hardware has the ability to switch on the Credit/Reset button (requires Rev 4 or greater)
  // then that can be used to choose Original or New code. Otherwise, the hardware switch
  // will choose Original if open, and New if closed
  initResult = RPU_InitializeMPU(   RPU_CMD_BOOT_ORIGINAL_IF_CREDIT_RESET | /*RPU_CMD_BOOT_ORIGINAL_IF_SWITCH_CLOSED |*/
                                    RPU_CMD_INIT_AND_RETURN_EVEN_IF_ORIGINAL_CHOSEN | RPU_CMD_PERFORM_MPU_TEST, SW_CREDIT_RESET);

  if (DEBUG_MESSAGES) {
    char buf[128];
    sprintf(buf, "Return from init = 0x%04lX\n", initResult);
    Serial.write(buf);
    if (initResult & RPU_RET_6800_DETECTED) Serial.write("Detected 6800 clock\n");
    else if (initResult & RPU_RET_6802_OR_8_DETECTED) Serial.write("Detected 6802/8 clock\n");
    Serial.write("Back from init\n");
  }

  if (initResult & RPU_RET_SELECTOR_SWITCH_ON) QueueDIAGNotification(SOUND_EFFECT_DIAG_SELECTOR_SWITCH_ON);
  else QueueDIAGNotification(SOUND_EFFECT_DIAG_SELECTOR_SWITCH_OFF);

  if (initResult & RPU_RET_CREDIT_RESET_BUTTON_HIT) QueueDIAGNotification(SOUND_EFFECT_DIAG_CREDIT_RESET_BUTTON);

  if (initResult & RPU_RET_DIAGNOSTIC_REQUESTED) {
    QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_DIAGNOSTICS);
    // Run diagnostics here:
  }

  if (initResult & RPU_RET_ORIGINAL_CODE_REQUESTED) {
    if (DEBUG_MESSAGES) Serial.write("Asked to run original code\n");
    delay(100);
    QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_ORIGINAL_CODE);
    delay(100);
    while (Audio.Update(millis()));
    // Arduino should hang if original code is running
    while (1);
  }
  QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_NEW_CODE);

  RPU_DisableSolenoidStack();
  RPU_SetDisableFlippers(true);

  // Read parameters from EEProm
  ReadStoredParameters();

  CurrentScores[0] = 0;
  CurrentScores[1] = 0;
  CurrentScores[2] = 0;
  CurrentScores[3] = 0;
  CurrentAchievements[0] = 0;
  CurrentAchievements[1] = 0;
  CurrentAchievements[2] = 0;
  CurrentAchievements[3] = 0;
  Display_EnableAchievements();
  CurrentTime = millis();

  MachineLocks = 0;

  Audio.QueueSound(SOUND_EFFECT_STARTUP_1 + (micros()) % 2, AUDIO_PLAY_TYPE_WAV_TRIGGER, CurrentTime + 1200);
  OperatorSwitchPressStarted = 0;
  InOperatorMenu = false;
  Menus.SetNavigationButtons(SW_RIGHT_FLIPPER, SW_HIGH_SCORE_RESET, SW_CREDIT_RESET, SW_SELF_TEST_SWITCH);
  Menus.SetMenuButtonDebounce(500);
  Menus.SetLampsLookupCallback(LampConvertDisplayNumberToIndex);
  Menus.SetSolenoidIDLookupCallback(SolenoidConvertDisplayNumberToIndex);
  Menus.SetSolenoidStrengthLookupCallback(SolenoidConvertDisplayNumberToTestStrength);
  Menus.SetSoundCallbackFunction(FPSoundTestCallback);
  BallRampKicked = 0;
  for (byte count=0; count<3; count++) LastSaucerEjectTime[count] = 0;
  for (byte count=0; count<3; count++) LastCoinSwitchTime[count] = 0;
  LastStartButtonSwitchTime = 0;

  Audio.PlaySoundCardWhenPossible(31 * 256, CurrentTime+3500, 0, 500, 10);

}



byte ReadSetting(byte setting, byte defaultValue, byte maxValue) {
  byte value = EEPROM.read(setting);
  if (value == 0xFF || value>maxValue) {
    EEPROM.write(setting, defaultValue);
    return defaultValue;
  }
  return value;
}

// This function is useful for checking the status of drop target switches
byte CheckSequentialSwitches(byte startingSwitch, byte numSwitches) {
  byte returnSwitches = 0;
  for (byte count = 0; count < numSwitches; count++) {
    returnSwitches |= (RPU_ReadSingleSwitchState(startingSwitch + count) << count);
  }
  return returnSwitches;
}


////////////////////////////////////////////////////////////////////////////
//
//  Lamp Management functions
//    These functions are called each time through the gameplay loop.
//    They use the current status variables to set each lamp to 
//    on, off, dim (depending on hardware), or flashing. The lamps are
//    actually set in hardware by the Interrupt Service Routine, so
//    these functions simply update the state arrays used by the ISR.
//
//    If a special animation is required (sweeps and such), then these
//    functions are turned off and one of the special functions in
//    "LampAnimations.h" is used to set those lamps.
//
////////////////////////////////////////////////////////////////////////////
void SetGeneralIlluminationOn(boolean setGIOn = true) {
  // Since this machine doesn't have GI control,
  // this line prevents compiler warnings.
  (void)setGIOn;
}


void ShowPlayerLamps() {
  
  for (byte count = 0; count < 4; count++) {
    if (count==CurrentPlayer) RPU_SetLampState(PlayerUpLamps[count], 1, 0, 250);
    else if (count<CurrentNumPlayers) RPU_SetLampState(PlayerUpLamps[count], 1);
    else RPU_SetLampState(PlayerUpLamps[count], 0);
    
    RPU_SetLampState(LAMP_HEAD_1_PLAYER+count, count==(CurrentNumPlayers-1));
  }  
}


void ShowHeadAndApronLamps() {
  if (MachineState==MACHINE_STATE_ATTRACT) {
    for (byte count=0; count<4; count++) RPU_SetLampState(PlayerUpLamps[count], 0);
    for (byte count=0; count<4; count++) RPU_SetLampState(LAMP_HEAD_1_PLAYER+count, 0);
    RPU_SetLampState(LAMP_HEAD_BALL_IN_PLAY, 0);
    RPU_SetLampState(LAMP_APRON_CREDITS, (Credits || FreePlayMode));
    RPU_SetLampState(LAMP_HEAD_TILT, (NumTiltWarnings > MaxTiltWarnings));
    RPU_SetLampState(LAMP_HEAD_GAME_OVER, 1);
    RPU_SetLampState(LAMP_HEAD_SHOOT_AGAIN, 0);
  } else {
    ShowPlayerLamps();
    if (MachineState!=MACHINE_STATE_MATCH_MODE) RPU_SetLampState(LAMP_HEAD_MATCH, 0);
    RPU_SetLampState(LAMP_HEAD_BALL_IN_PLAY, 1, 0, 150);
    RPU_SetLampState(LAMP_APRON_CREDITS, (Credits || FreePlayMode));
    RPU_SetLampState(LAMP_HEAD_TILT, (NumTiltWarnings > MaxTiltWarnings));
    RPU_SetLampState(LAMP_HEAD_GAME_OVER, 0);
    RPU_SetLampState(LAMP_HEAD_HIGH_SCORE, 0);
    //RPU_SetLampState(LAMP_HEAD_SHOOT_AGAIN, 0); This is handled by ShowShootAgain function
  }
}

byte BonusLampAssignments[11] = { 
    LAMP_BONUS_1, LAMP_BONUS_2, LAMP_BONUS_3, LAMP_BONUS_4, LAMP_BONUS_5, LAMP_BONUS_6,
    LAMP_BONUS_7, LAMP_BONUS_8, LAMP_BONUS_9, LAMP_BONUS_10, LAMP_BONUS_20
    };

void ShowBonusLamps() {

  if (GameMode == GAME_MODE_SKILL_SHOT) {
    byte lampPhase = (CurrentTime / 100)%11;
    for (byte count=0; count<11; count++) {
      RPU_SetLampState(BonusLampAssignments[count], count==lampPhase);
    }
  } else if (GameMode == GAME_MODE_BOSS_BATTLE) {
    byte ladderPhase = (CurrentTime/75)%20;
    if (ladderPhase>=11) ladderPhase = 20-ladderPhase;
    for (byte count=0; count<11; count++) {
      RPU_SetLampState(BonusLampAssignments[count], count==ladderPhase);
    }
  } else if (MachineState==MACHINE_STATE_NORMAL_GAMEPLAY && BattleStage==BATTLE_STAGE_BULLSEYE) {
    byte lampPhase = (CurrentTime/50)%15;
    for (byte count=0; count<11; count++) {
      RPU_SetLampState(BonusLampAssignments[count], count==lampPhase);
    }
  } else {
    byte remainderBonus = Bonus[CurrentPlayer]%10;
    byte bonus10 = (Bonus[CurrentPlayer]/10)%2;
    byte bonus20 = (Bonus[CurrentPlayer]>=20) ? 1 : 0;

    for (byte count=0; count<10; count++) {
      RPU_SetLampState(BonusLampAssignments[count], remainderBonus>count);
    }
    RPU_SetLampState(LAMP_BONUS_10, bonus10);
    RPU_SetLampState(LAMP_BONUS_20, bonus20);
  }

  // bonus X
  switch (BonusX[CurrentPlayer]) {
    case 0:
    case 1:
      RPU_SetLampState(LAMP_BONUS_2X, 0);
      RPU_SetLampState(LAMP_BONUS_3X, 0);
      RPU_SetLampState(LAMP_BONUS_4X, 0);
      RPU_SetLampState(LAMP_BONUS_5X, 0);
      break;
    case 2:
      RPU_SetLampState(LAMP_BONUS_2X, 1);
      RPU_SetLampState(LAMP_BONUS_3X, 0);
      RPU_SetLampState(LAMP_BONUS_4X, 0);
      RPU_SetLampState(LAMP_BONUS_5X, 0);
      break;
    case 3:
      RPU_SetLampState(LAMP_BONUS_2X, 0);
      RPU_SetLampState(LAMP_BONUS_3X, 1);
      RPU_SetLampState(LAMP_BONUS_4X, 0);
      RPU_SetLampState(LAMP_BONUS_5X, 0);
      break;
    case 4:
      RPU_SetLampState(LAMP_BONUS_2X, 0);
      RPU_SetLampState(LAMP_BONUS_3X, 0);
      RPU_SetLampState(LAMP_BONUS_4X, 1);
      RPU_SetLampState(LAMP_BONUS_5X, 0);
      break;
    case 5:
      RPU_SetLampState(LAMP_BONUS_2X, 0);
      RPU_SetLampState(LAMP_BONUS_3X, 0);
      RPU_SetLampState(LAMP_BONUS_4X, 0);
      RPU_SetLampState(LAMP_BONUS_5X, 1);
      break;
    case 6:
      RPU_SetLampState(LAMP_BONUS_2X, 1);
      RPU_SetLampState(LAMP_BONUS_3X, 0);
      RPU_SetLampState(LAMP_BONUS_4X, 1);
      RPU_SetLampState(LAMP_BONUS_5X, 0);
      break;
    case 7:
      RPU_SetLampState(LAMP_BONUS_2X, 1);
      RPU_SetLampState(LAMP_BONUS_3X, 0);
      RPU_SetLampState(LAMP_BONUS_4X, 0);
      RPU_SetLampState(LAMP_BONUS_5X, 1);
      break;
    case 8:
      RPU_SetLampState(LAMP_BONUS_2X, 0);
      RPU_SetLampState(LAMP_BONUS_3X, 1);
      RPU_SetLampState(LAMP_BONUS_4X, 0);
      RPU_SetLampState(LAMP_BONUS_5X, 1);
      break;
    case 9:
      RPU_SetLampState(LAMP_BONUS_2X, 0);
      RPU_SetLampState(LAMP_BONUS_3X, 0);
      RPU_SetLampState(LAMP_BONUS_4X, 1);
      RPU_SetLampState(LAMP_BONUS_5X, 1);
      break;
  }

}


byte PopBumperLampIndices[] = {LAMP_BOTTOM_LEFT_POP, LAMP_TOP_LEFT_POP, LAMP_TOP_RIGHT_POP, LAMP_BOTTOM_RIGHT_POP};
void ShowPopBumperLamps() {

  if (GameMode==GAME_MODE_BOSS_BATTLE) {
    byte stageIndex = BossBattleStage - 1;
    byte stageMask = 0x01 << stageIndex;

    if (BossBattleStage==BOSS_STAGE_POP_BUMPERS) {
      byte hitsDone = BossHitsInStage[stageIndex];
      byte hitsNeeded = BossBattleShotThreshold[CurrentPlayer];
      if (BossBattleStagesDone & stageMask) {
        RPU_SetLampState(LAMP_BOTTOM_LEFT_POP, 1, 0, 10);
        RPU_SetLampState(LAMP_TOP_RIGHT_POP, 1, 0, 10);
        RPU_SetLampState(LAMP_TOP_LEFT_POP, 1, 0, 10);
        RPU_SetLampState(LAMP_BOTTOM_RIGHT_POP, 1, 0, 10);
      } else {
        int flashSpeed = 50;
        if (hitsNeeded>hitsDone) flashSpeed += ((int)(hitsNeeded-hitsDone) * 50);
        if (flashSpeed>1000) flashSpeed = 1000;
        RPU_SetLampState(LAMP_BOTTOM_LEFT_POP, 1, 0, flashSpeed);
        RPU_SetLampState(LAMP_TOP_RIGHT_POP, 1, 0, flashSpeed);
        RPU_SetLampState(LAMP_TOP_LEFT_POP, 1, 0, flashSpeed);
        RPU_SetLampState(LAMP_BOTTOM_RIGHT_POP, 1, 0, flashSpeed);
      }
    } else {
      if (BossBattleStagesDone & stageMask) {
        RPU_SetLampState(LAMP_BOTTOM_LEFT_POP, 1);
        RPU_SetLampState(LAMP_TOP_RIGHT_POP, 1);
        RPU_SetLampState(LAMP_TOP_LEFT_POP, 1);
        RPU_SetLampState(LAMP_BOTTOM_RIGHT_POP, 1);
      } else {
        RPU_SetLampState(LAMP_BOTTOM_LEFT_POP, 0);
        RPU_SetLampState(LAMP_TOP_RIGHT_POP, 0);
        RPU_SetLampState(LAMP_TOP_LEFT_POP, 0);
        RPU_SetLampState(LAMP_BOTTOM_RIGHT_POP, 0);
      }
    }
  } else if ((GameMode==GAME_MODE_UNSTRUCTURED_PLAY || GameMode==GAME_MODE_SKILL_SHOT) && PlayerTrainingStatus[CurrentPlayer] & WEAPONS_TRAINING_QUALIFIED) {
    byte popPhase = ((CurrentTime/600)%3);
    RPU_SetLampState(LAMP_BOTTOM_LEFT_POP, popPhase==0, 0, 100);
    RPU_SetLampState(LAMP_TOP_RIGHT_POP, popPhase==0, 0, 100);
    RPU_SetLampState(LAMP_TOP_LEFT_POP, popPhase==1, 0, 100);
    RPU_SetLampState(LAMP_BOTTOM_RIGHT_POP, popPhase==1, 0, 100);
  } else if (PlayerTrainingStatus[CurrentPlayer] & WEAPONS_TRAINING_RUNNING) {
    byte popPhase = ((CurrentTime/100)%4);
    RPU_SetLampState(LAMP_TOP_LEFT_POP, popPhase==0);
    RPU_SetLampState(LAMP_TOP_RIGHT_POP, popPhase==1);
    RPU_SetLampState(LAMP_BOTTOM_RIGHT_POP, popPhase==2);
    RPU_SetLampState(LAMP_BOTTOM_LEFT_POP, popPhase==3);
  } else if (PlayerTrainingStatus[CurrentPlayer] & WEAPONS_TRAINING_ACHIEVED_MASK) {
    byte popPhase = ((CurrentTime/100)%3);
    byte trainingLevel = (PlayerTrainingStatus[CurrentPlayer] & WEAPONS_TRAINING_ACHIEVED_MASK)/WEAPONS_TRAINING_ACHIEVED_SHIFT;
    RPU_SetLampState(LAMP_TOP_LEFT_POP, trainingLevel>popPhase);
    RPU_SetLampState(LAMP_TOP_RIGHT_POP, trainingLevel>popPhase);
    RPU_SetLampState(LAMP_BOTTOM_RIGHT_POP, trainingLevel>popPhase);
    RPU_SetLampState(LAMP_BOTTOM_LEFT_POP, trainingLevel>popPhase);
  } else {
    for (byte count=0; count<4; count++) {
      if (LastTimePopHit[count]) {
        RPU_SetLampState(PopBumperLampIndices[count], 1, 0, 75);
        if (CurrentTime>(LastTimePopHit[count]+500)) LastTimePopHit[count] = 0;
      } else {
        RPU_SetLampState(PopBumperLampIndices[count], 0);
      }
    }    
  }

}


void ShowShootAgainLamp() {

  if ( (BallFirstSwitchHitTime == 0 && BallSaveNumSeconds) || (BallSaveEndTime && CurrentTime < BallSaveEndTime) ) {
    unsigned long msRemaining = 5000;
    if (BallSaveEndTime != 0) msRemaining = BallSaveEndTime - CurrentTime;
    RPU_SetLampState(LAMP_SHOOT_AGAIN, 1, 0, (msRemaining < 5000) ? 100 : 500);
    RPU_SetLampState(LAMP_HEAD_SHOOT_AGAIN, 1, 0, (msRemaining < 5000) ? 100 : 500);
  } else {
    RPU_SetLampState(LAMP_SHOOT_AGAIN, SamePlayerShootsAgain);
    RPU_SetLampState(LAMP_HEAD_SHOOT_AGAIN, SamePlayerShootsAgain);
  }
}


void ShowFireLamps() {
  if (GameMode==GAME_MODE_SKILL_SHOT) {
    if (SkillShotType[CurrentPlayer]!=SKILL_SHOT_TYPE_AUTO_CHANGING_LANE || RPU_ReadSingleSwitchState(SW_RIGHT_FLIPPER)) {
      RPU_SetLampState(LAMP_TOP_F, SkillShotLane==0, 0, 200);
      RPU_SetLampState(LAMP_TOP_I, SkillShotLane==1, 0, 200);
      RPU_SetLampState(LAMP_TOP_R, SkillShotLane==2, 0, 200);
      RPU_SetLampState(LAMP_TOP_E, SkillShotLane==3, 0, 200);
    } else {
      RPU_SetLampState(LAMP_TOP_F, SkillShotLane==0);
      RPU_SetLampState(LAMP_TOP_I, SkillShotLane==1);
      RPU_SetLampState(LAMP_TOP_R, SkillShotLane==2);
      RPU_SetLampState(LAMP_TOP_E, SkillShotLane==3);
    }
    RPU_SetLampState(LAMP_CENTER_FIRE, 0);
  } else if (GameMode==GAME_MODE_BOSS_BATTLE) {
    byte stageIndex = BossBattleStage - 1;
    byte stageMask = 0x01 << stageIndex;

    if (BossBattleStage==BOSS_STAGE_FIRE_LANES) {
      byte hitsDone = BossHitsInStage[stageIndex];
      byte hitsNeeded = BossBattleShotThreshold[CurrentPlayer];
      if (BossBattleStagesDone & stageMask) {
        RPU_SetLampState(LAMP_TOP_F, 1, 0, 10);
        RPU_SetLampState(LAMP_TOP_I, 1, 0, 10);
        RPU_SetLampState(LAMP_TOP_R, 1, 0, 10);
        RPU_SetLampState(LAMP_TOP_E, 1, 0, 10);
      } else {
        int flashSpeed = 50;
        if (hitsNeeded>hitsDone) flashSpeed += ((int)(hitsNeeded-hitsDone) * 50);
        if (flashSpeed>1000) flashSpeed = 1000;
        RPU_SetLampState(LAMP_TOP_F, 1, 0, flashSpeed);
        RPU_SetLampState(LAMP_TOP_I, 1, 0, flashSpeed);
        RPU_SetLampState(LAMP_TOP_R, 1, 0, flashSpeed);
        RPU_SetLampState(LAMP_TOP_E, 1, 0, flashSpeed);
      }
    } else {
      if (BossBattleStagesDone & stageMask) {
        RPU_SetLampState(LAMP_TOP_F, 1);
        RPU_SetLampState(LAMP_TOP_I, 1);
        RPU_SetLampState(LAMP_TOP_R, 1);
        RPU_SetLampState(LAMP_TOP_E, 1);
      } else {
        RPU_SetLampState(LAMP_TOP_F, 0);
        RPU_SetLampState(LAMP_TOP_I, 0);
        RPU_SetLampState(LAMP_TOP_R, 0);
        RPU_SetLampState(LAMP_TOP_E, 0);
      }
    }
  } else {
    byte fireFlag = 0x01;
    for (byte count=0; count<4; count++) {
      if (FireLaneHitTime[count]) RPU_SetLampState(LAMP_TOP_F + count, 1, 0, 75);
      else RPU_SetLampState(LAMP_TOP_F + count, FireStatus[CurrentPlayer] & fireFlag);
      fireFlag *= 2;
    }

    if (FirePowerLevelChangedTime) {
      byte currentLightShowIndex = 0;
      for (byte count=0; count<10; count++) {
        if ( CurrentTime > (FirePowerLevelChangedTime + ((unsigned long)FirePowerIncreaseLampOffsets[count])*8)) currentLightShowIndex = count;
        else break;
      }
      if (currentLightShowIndex<9) RPU_SetLampState(LAMP_CENTER_FIRE, (currentLightShowIndex%2)==0);
      else RPU_SetLampState(LAMP_CENTER_FIRE, 1, 0, 50);
    } else if (FireCompletedTime) {
      RPU_SetLampState(LAMP_CENTER_FIRE, 1, 0, 100);
    } else if (FireCompletions[CurrentPlayer]>PowerCompletions[CurrentPlayer]) {
      RPU_SetLampState(LAMP_CENTER_FIRE, 1);
    } else {
      RPU_SetLampState(LAMP_CENTER_FIRE, 0);
    }  
  }
}

byte LockLamps[] = {LAMP_LEFT_SAUCER, LAMP_TOP_SAUCER, LAMP_RIGHT_SAUCER};

void ShowLockLamps() {

  if (GameMode==GAME_MODE_UNSTRUCTURED_PLAY || GameMode==GAME_MODE_SKILL_SHOT) {
    byte qualifiedBit = BALL_LEFT_LOCK_QUALIFIED;
    byte lockBit = BALL_LEFT_LOCK_ENGAGED;
    for (byte count=0; count<3; count++) {
      int flashSpeed = 0;
      if (PlayerLocks[CurrentPlayer] & qualifiedBit) {
        if ( (MachineLocks & lockBit) ) flashSpeed = 750;
        else flashSpeed = 250;
      }
      if (count==OfferBattleSaucer) {
        flashSpeed = 50;
      }
      boolean lockOrBattle = ((PlayerLocks[CurrentPlayer]&lockBit) || flashSpeed);
      if (lockOrBattle) {
        RPU_SetLampState(LockLamps[count], 1, 0, flashSpeed);
      } else {
        // If this lock is not currently being used for somethiing else, it could be 
        // used for starting a qualified training if we're in GAME_MODE_UNSTRUCTURED_PLAY
        if (PlayerTrainingStatus[CurrentPlayer] & TRAINING_QUALIFIED_MASK) {
          byte qualifiedPhase = ((CurrentTime/600)%3);
          RPU_SetLampState(LockLamps[count], qualifiedPhase==2, 0, 100);
        }
      }
      
      qualifiedBit *= 2;
      lockBit *= 2;
    }
  } else if (GameMode==GAME_MODE_BOSS_BATTLE) {
    for (byte count=0; count<3; count++) {
      if (BossSaucerKickoutTime[count]) {
        if ( (CurrentTime+1000)>BossSaucerKickoutTime[count] ) {
          // This saucer will kick in the next second
          RPU_SetLampState(LockLamps[count], 1, 0, 75);
        } else {
          // This saucer is holding a ball
          RPU_SetLampState(LockLamps[count], 1, 0, 300);
        }
      } else {
        RPU_SetLampState(LockLamps[count], 0);
      }
    }
  } else if (BattleStage!=BATTLE_STAGE_OFF) {
    for (byte count=0; count<3; count++) {
      if (GameMode==GAME_MODE_BATTLE_1 && count==OfferBattleSaucer) {
        RPU_SetLampState(LockLamps[count], 1, 0, 1000);
      } else if (JackpotReady) {
        RPU_SetLampState(LockLamps[count], 1, 0, 75);
      } else {
        RPU_SetLampState(LockLamps[count], 0);
      }
    }    
  } else {
    for (byte count=0; count<3; count++) {
      RPU_SetLampState(LockLamps[count], 0);
    }
  }

  
  
}

byte TargetLamps[] = {LAMP_1_STANDUP, LAMP_2_STANDUP, LAMP_3_STANDUP, LAMP_4_STANDUP, LAMP_5_STANDUP, LAMP_6_STANDUP};
byte CenterAwardLamps[] = {LAMP_10K_FIREPOWER, LAMP_30K_FIREPOWER, LAMP_50K_FIREPOWER};

void ShowTargetLamps() {

  if (GameMode==GAME_MODE_BOSS_BATTLE) {
    byte stageIndex = BossBattleStage - 1;
    byte stageMask = 0x01 << stageIndex;

    if (BossBattleStage==BOSS_STAGE_NUMBER_STANDUPS) {
      byte hitsDone = BossHitsInStage[stageIndex];
      byte hitsNeeded = BossBattleShotThreshold[CurrentPlayer];
      if (BossBattleStagesDone & stageMask) {
        for (byte count=0; count<6; count++) RPU_SetLampState(TargetLamps[count], 1, 0, 10);
      } else {
        int flashSpeed = 50;
        if (hitsNeeded>hitsDone) flashSpeed += ((int)(hitsNeeded-hitsDone) * 50);
        if (flashSpeed>1000) flashSpeed = 1000;
        for (byte count=0; count<6; count++) RPU_SetLampState(TargetLamps[count], 1, 0, flashSpeed);
      }
    } else {
      if (BossBattleStagesDone & stageMask) {
        for (byte count=0; count<6; count++) RPU_SetLampState(TargetLamps[count], 1);
      } else {
        for (byte count=0; count<6; count++) RPU_SetLampState(TargetLamps[count], 0);
      }
    }
    for (byte count=0; count<3; count++) RPU_SetLampState(CenterAwardLamps[count], 0);
  } else if (GameMode==GAME_MODE_SKILL_SHOT || (BattleStage!=BATTLE_STAGE_OFF && BattleStage!=BATTLE_STAGE_STANDUPS && BattleStage!=BATTLE_STAGE_BULLSEYE)) {
    // turn off these lamps when you're in skill shot, or you have a battle stage that's not standups or bullseye
    for (byte count=0; count<6; count++) RPU_SetLampState(TargetLamps[count], 0);
    for (byte count=0; count<3; count++) RPU_SetLampState(CenterAwardLamps[count], 0);
  } else if (BattleStage==BATTLE_STAGE_STANDUPS) {
    byte standupBit = 0x01;
    for (byte count=0; count<6; count++) {
      if (LastBattleStandupHit==count && LastBattleStandupHitTime) {
        RPU_SetLampState(TargetLamps[count], 1, 0, 100);
      } else {
        boolean lampOn = false;
        if (standupBit & BattleStandups) lampOn = true;
        RPU_SetLampState(TargetLamps[count], lampOn);
      }
      standupBit *= 2;
    }
    for (byte count=0; count<3; count++) RPU_SetLampState(CenterAwardLamps[count], 0);
  } else if (BattleStage==BATTLE_STAGE_BULLSEYE) {
    byte lampPhase = (CurrentTime/50)%15;
    for (byte count=0; count<3; count++) {
      if (lampPhase>=9 && lampPhase<=11) {
        RPU_SetLampState(TargetLamps[count], count==(lampPhase-9));
        RPU_SetLampState(TargetLamps[5 - count], count==(lampPhase-9));
      } else {
        RPU_SetLampState(TargetLamps[count], 0);
        RPU_SetLampState(TargetLamps[5 - count], 0);
      }
    }
    for (byte count=0; count<3; count++) {
      RPU_SetLampState(CenterAwardLamps[count], count==(lampPhase-12));
    }    
  } else {
    byte lampPhase = (CurrentTime/75)%3;
    byte targetBit = 0x01;
    for (byte count=0; count<6; count++) {
      if (count==3) targetBit = 0x10;
      byte lampOn = false;
      if (StandupTargetStatus[CurrentPlayer] & targetBit) {
        lampOn = true;
      } else {
        if (count<3 && lampPhase!=count) lampOn = true;
        else if (count>=3 && lampPhase!=(5-count)) lampOn = true;
      }
      RPU_SetLampState(TargetLamps[count], lampOn);
      targetBit *= 2;
    }

    RPU_SetLampState(LAMP_10K_FIREPOWER, FirePowerLevel[CurrentPlayer], 0, FirePowerLevelChangedTime?100:0);
    RPU_SetLampState(LAMP_30K_FIREPOWER, FirePowerLevel[CurrentPlayer]>1, 0, FirePowerLevelChangedTime?100:0);
    RPU_SetLampState(LAMP_50K_FIREPOWER, FirePowerLevel[CurrentPlayer]>2, 0, FirePowerLevelChangedTime?100:0);
  }
}

void ShowPowerLamps() {
  if (GameMode==GAME_MODE_SKILL_SHOT) {
    RPU_SetLampState(LAMP_POWER_1, 0);
    RPU_SetLampState(LAMP_POWER_2, 0);
    RPU_SetLampState(LAMP_POWER_3, 0);
    RPU_SetLampState(LAMP_CENTER_POWER, 0);
    RPU_SetLampState(LAMP_EXTRA_BALL, 0);
  } else if (GameMode==GAME_MODE_BOSS_BATTLE) {
    byte stageIndex = BossBattleStage - 1;
    byte stageMask = 0x01 << stageIndex;

    if (BossBattleStage==BOSS_STAGE_POWER_STANDUPS) {
      byte hitsDone = BossHitsInStage[stageIndex];
      byte hitsNeeded = BossBattleShotThreshold[CurrentPlayer];
      if (BossBattleStagesDone & stageMask) {
        RPU_SetLampState(LAMP_POWER_1, 1, 0, 10);
        RPU_SetLampState(LAMP_POWER_2, 1, 0, 10);
        RPU_SetLampState(LAMP_POWER_3, 1, 0, 10);
      } else {
        int flashSpeed = 50;
        if (hitsNeeded>hitsDone) flashSpeed += ((int)(hitsNeeded-hitsDone) * 50);
        if (flashSpeed>1000) flashSpeed = 1000;
        RPU_SetLampState(LAMP_POWER_1, 1, 0, flashSpeed);
        RPU_SetLampState(LAMP_POWER_2, 1, 0, flashSpeed);
        RPU_SetLampState(LAMP_POWER_3, 1, 0, flashSpeed);
      }
    } else {
      if (BossBattleStagesDone & stageMask) {
        RPU_SetLampState(LAMP_POWER_1, 1);
        RPU_SetLampState(LAMP_POWER_2, 1);
        RPU_SetLampState(LAMP_POWER_3, 1);
      } else {
        RPU_SetLampState(LAMP_POWER_1, 0);
        RPU_SetLampState(LAMP_POWER_2, 0);
        RPU_SetLampState(LAMP_POWER_3, 0);
      }
    }
    RPU_SetLampState(LAMP_CENTER_POWER, 0);
    RPU_SetLampState(LAMP_EXTRA_BALL, 0);    
  } else {
    if (ComboCompletesPower && LeftInlaneStage) {
      RPU_SetLampState(LAMP_POWER_1, 1, 0, 125);
      RPU_SetLampState(LAMP_POWER_2, 1, 0, 125);
      RPU_SetLampState(LAMP_POWER_3, 1, 0, 125);
    } else if (PowerCompletedTime==0) {
      byte powerFlag = 0x01;
      for (byte count=0; count<3; count++) {
        if (CurrentTime<(PowerTargetHitTime[count]+1500)) {
          RPU_SetLampState(LAMP_POWER_1+count, 1, 0, 75);
        } else {
          RPU_SetLampState(LAMP_POWER_1+count, PowerStatus[CurrentPlayer] & powerFlag);
        }
        powerFlag *= 2;
      }
      RPU_SetLampState(LAMP_POWER_1, PowerStatus[CurrentPlayer] & 0x01);
      RPU_SetLampState(LAMP_POWER_2, PowerStatus[CurrentPlayer] & 0x02);
      RPU_SetLampState(LAMP_POWER_3, PowerStatus[CurrentPlayer] & 0x04);
    } else {
      byte lampPhase = (CurrentTime/150)%3;
      RPU_SetLampState(LAMP_POWER_1, lampPhase==0);
      RPU_SetLampState(LAMP_POWER_2, lampPhase==1);
      RPU_SetLampState(LAMP_POWER_3, lampPhase==2);
    }
    RPU_SetLampState(LAMP_EXTRA_BALL, 0);

    if (FirePowerLevelChangedTime) {
      byte currentLightShowIndex = 0;
      for (byte count=0; count<10; count++) {
        if ( CurrentTime > (FirePowerLevelChangedTime + ((unsigned long)FirePowerIncreaseLampOffsets[count])*8)) currentLightShowIndex = count;
        else break;
      }
      if (currentLightShowIndex<9) RPU_SetLampState(LAMP_CENTER_POWER, (currentLightShowIndex%2)==1);
      else RPU_SetLampState(LAMP_CENTER_POWER, 1, 0, 50);
    } else if (PowerCompletedTime) {
      RPU_SetLampState(LAMP_CENTER_POWER, 1, 0, 100);
    } else if (PowerCompletions[CurrentPlayer]>FireCompletions[CurrentPlayer]) {
      RPU_SetLampState(LAMP_CENTER_POWER, 1);
    } else {
      RPU_SetLampState(LAMP_CENTER_POWER, 0);
    }
  }  
}

void ShowLaneLamps() {
  if (GameMode==GAME_MODE_SKILL_SHOT) {
    RPU_SetLampState(LAMP_SPINNER, 0);
    RPU_SetLampState(LAMP_LEFT_INLANE, 0);
    RPU_SetLampState(LAMP_RIGHT_INLANE, 0);
    RPU_SetLampState(LAMP_LEFT_OUTLANE, 0);
    RPU_SetLampState(LAMP_RIGHT_OUTLANE, 0);
    RPU_SetLampState(LAMP_BALL_SAVER_KICKER, 0);
  } else {
    
    if (RightInlaneStage) {
      int flashRate = 750 / RightInlaneStage;
      RPU_SetLampState(LAMP_SPINNER, 1, 0, flashRate);
    } else {
      if (BattleStage==BATTLE_STAGE_SPINNER) {
        RPU_SetLampState(LAMP_SPINNER, 1, 0, 125);
      } else if ( PlayerTrainingStatus[CurrentPlayer] & FLIGHT_TRAINING_QUALIFIED ) {
        byte spinnerLampPhase = ((CurrentTime/600)%3);
        RPU_SetLampState(LAMP_SPINNER, spinnerLampPhase!=2, 0, 100);
      } else if ( PlayerTrainingStatus[CurrentPlayer] & FLIGHT_TRAINING_RUNNING) {
        RPU_SetLampState(LAMP_SPINNER, 1, 0, 100);
      } else if ( PlayerTrainingStatus[CurrentPlayer] & FLIGHT_TRAINING_ACHIEVED_MASK ) {
        byte spinnerLampPhase = ((CurrentTime/100)%3);
        byte trainingLevel = (PlayerTrainingStatus[CurrentPlayer] & FLIGHT_TRAINING_ACHIEVED_MASK)/FLIGHT_TRAINING_ACHIEVED_SHIFT;
        RPU_SetLampState(LAMP_SPINNER, trainingLevel>spinnerLampPhase);
      } else {
        RPU_SetLampState(LAMP_SPINNER, 0);
      }
    }
    int leftFlash = 1500 / (LeftInlaneStage+1);
    int rightFlash = 1500 / (RightInlaneStage+1);
    RPU_SetLampState(LAMP_LEFT_INLANE, LeftInlaneStage, 0, leftFlash);
    RPU_SetLampState(LAMP_RIGHT_INLANE, RightInlaneStage, 0, rightFlash);
    RPU_SetLampState(LAMP_LEFT_OUTLANE, 0);
    RPU_SetLampState(LAMP_RIGHT_OUTLANE, 0);
    RPU_SetLampState(LAMP_BALL_SAVER_KICKER, (LeftKickback[CurrentPlayer])?1:0, 0, (BallKickerEndTime)?50:0);
  }
}


////////////////////////////////////////////////////////////////////////////
//
//  Machine State Helper functions
//
////////////////////////////////////////////////////////////////////////////
boolean AddPlayer(boolean resetNumPlayers = false) {

  if (Credits < 1 && !FreePlayMode) return false;
  if (resetNumPlayers) CurrentNumPlayers = 0;
  if (CurrentNumPlayers >= RPU_NUMBER_OF_PLAYERS_ALLOWED) return false;

  if (CurrentNumPlayers==0) {
    PlayerNickname[0] = (CurrentTime%NICKNAMES_LEVEL_1_QTY); // pick a random nickname for player 1
  } else {
    // advance through the nicknames for each successive player
    PlayerNickname[CurrentNumPlayers] = (PlayerNickname[CurrentNumPlayers-1] + 1)%NICKNAMES_LEVEL_1_QTY;
  }

  CurrentNumPlayers += 1;
  RPU_SetDisplay(CurrentNumPlayers - 1, 0, true, 2);
  
  if (CurrentNumPlayers > 1) {
    // If this is second, third, or fourth player, then playe the announcment
//    QueueNotification(SOUND_EFFECT_VP_ADD_PLAYER_1 + (CurrentNumPlayers - 1), 10);
    PlaySoundEffect(SOUND_EFFECT_VP_ADD_PLAYER_1 + (CurrentNumPlayers - 1));
  }

  for (byte count = 0; count < 4; count++) {
    if (count==CurrentPlayer) RPU_SetLampState(PlayerUpLamps[count], 1, 0, 250);
    else if (count<CurrentNumPlayers) RPU_SetLampState(PlayerUpLamps[count], 1);
    else RPU_SetLampState(PlayerUpLamps[count], 0);
  }

  if (!FreePlayMode) {
    Credits -= 1;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    if (!FreePlayMode) RPU_SetCoinLockout(false, SOLCONT_COIN_LOCKOUT);
  }

  RPU_WriteULToEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE) + 1);

  return true;
}


unsigned short ChuteAuditByte[] = {RPU_CHUTE_1_COINS_START_BYTE, RPU_CHUTE_2_COINS_START_BYTE, RPU_CHUTE_3_COINS_START_BYTE};
void AddCoinToAudit(byte chuteNum) {
  if (chuteNum > 2) return;
  unsigned short coinAuditStartByte = ChuteAuditByte[chuteNum];
  RPU_WriteULToEEProm(coinAuditStartByte, RPU_ReadULFromEEProm(coinAuditStartByte) + 1);
}


void AddCredit(boolean playSound = false, byte numToAdd = 1) {
  if (Credits < MaximumCredits) {
    Credits += numToAdd;
    if (Credits > MaximumCredits) Credits = MaximumCredits;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    if (playSound) {
      //PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
      RPU_PushToSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, true);
    }
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    if (!FreePlayMode) RPU_SetCoinLockout(false, SOLCONT_COIN_LOCKOUT);
  } else {
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    if (!FreePlayMode) RPU_SetCoinLockout(true, SOLCONT_COIN_LOCKOUT);
    else RPU_SetCoinLockout(false, SOLCONT_COIN_LOCKOUT);
  }

}

byte SwitchToChuteNum(byte switchHit) {
  byte chuteNum = 0; // default to SW_COIN_1
  if (switchHit==SW_COIN_2) chuteNum = 1;
  if (switchHit==SW_COIN_3) chuteNum = 2;
  return chuteNum;
}

boolean AddCoin(byte chuteNum) {
  boolean creditAdded = false;
  if (chuteNum > 2) return false;
  byte cpcSelection = GetCPCSelection(chuteNum);

  // Find the lowest chute num with the same ratio selection
  // and use that ChuteCoinsInProgress counter
  byte chuteNumToUse;
  for (chuteNumToUse = 0; chuteNumToUse <= chuteNum; chuteNumToUse++) {
    if (GetCPCSelection(chuteNumToUse) == cpcSelection) break;
  }

  PlaySoundEffect(SOUND_EFFECT_COIN_DROP_1 + (CurrentTime % 3));

  byte cpcCoins = GetCPCCoins(cpcSelection);
  byte cpcCredits = GetCPCCredits(cpcSelection);
  byte coinProgressBefore = ChuteCoinsInProgress[chuteNumToUse];
  ChuteCoinsInProgress[chuteNumToUse] += 1;

  if (ChuteCoinsInProgress[chuteNumToUse] == cpcCoins) {
    if (cpcCredits > cpcCoins) AddCredit(cpcCredits - (coinProgressBefore));
    else AddCredit(cpcCredits);
    ChuteCoinsInProgress[chuteNumToUse] = 0;
    creditAdded = true;
  } else {
    if (cpcCredits > cpcCoins) {
      AddCredit(1);
      creditAdded = true;
    } else {
    }
  }

  return creditAdded;
}


void AddSpecialCredit() {
  AddCredit(false, 1);
  RPU_PushToTimedSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, CurrentTime, true);
  RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 1);
}

void AwardSpecial() {
  if (SpecialCollected) return;
  SpecialCollected = true;
  if (TournamentScoring) {
    CurrentScores[CurrentPlayer] += SpecialValue * PlayfieldMultiplier;
  } else {
    AddSpecialCredit();
  }
}

boolean AwardExtraBall(boolean basedOnScore = false) {
  if (ExtraBallAvailable[CurrentPlayer]==1 || (basedOnScore && ExtraBallAvailable[CurrentPlayer]!=2)) {
    ExtraBallAvailable[CurrentPlayer] = 2;
    if (TournamentScoring) {
      CurrentScores[CurrentPlayer] += ExtraBallValue * PlayfieldMultiplier;
    } else {
      SamePlayerShootsAgain = true;
      RPU_SetLampState(LAMP_SHOOT_AGAIN, SamePlayerShootsAgain);
      QueueNotification(SOUND_EFFECT_VP_EXTRA_BALL, 8);
    }
    return true;
  }
  return false;
}


void IncreasePlayfieldMultiplier(unsigned long duration) {
  if (PlayfieldMultiplierExpirationTime) {
    PlayfieldMultiplierExpirationTime += duration;
  } else {
    PlayfieldMultiplierExpirationTime = CurrentTime + duration;
  }

  PlayfieldMultiplier += 1;
  if (PlayfieldMultiplier > 5) {
    PlayfieldMultiplier = 5;
  }
}


void SetBallSave(unsigned long duration, byte numberOfSaves = 0xFF, boolean addToBallSave = false) {

  if (duration == 0) {
    BallSaveEndTime = 0;
    NumberOfBallSavesRemaining = 0;
  } else if (addToBallSave) {
    if (BallSaveEndTime) BallSaveEndTime += duration;
  } else {
    BallSaveEndTime = CurrentTime + duration;
    NumberOfBallSavesRemaining = numberOfSaves;
  }
}




#define SOUND_EFFECT_OM_CPC_VALUES                  180
#define SOUND_EFFECT_OM_CRB_VALUES                  210

#define SOUND_EFFECT_AP_TOP_LEVEL_MENU_ENTRY    1700
#define SOUND_EFFECT_AP_TEST_MENU               1701
#define SOUND_EFFECT_AP_AUDITS_MENU             1702
#define SOUND_EFFECT_AP_BASIC_ADJUSTMENTS_MENU  1703
#define SOUND_EFFECT_AP_GAME_RULES_LEVEL        1704
#define SOUND_EFFECT_AP_GAME_SPECIFIC_ADJ_MENU  1705

#define SOUND_EFFECT_AP_TEST_LAMPS              1710
#define SOUND_EFFECT_AP_TEST_DISPLAYS           1711
#define SOUND_EFFECT_AP_TEST_SOLENOIDS          1712
#define SOUND_EFFECT_AP_TEST_SWITCHES           1713
#define SOUND_EFFECT_AP_TEST_SOUNDS             1714
#define SOUND_EFFECT_AP_TEST_EJECT_BALLS        1715

#define SOUND_EFFECT_AP_AUDIT_TOTAL_PLAYS       1720
#define SOUND_EFFECT_AP_AUDIT_CHUTE_1_COINS     1721
#define SOUND_EFFECT_AP_AUDIT_CHUTE_2_COINS     1722
#define SOUND_EFFECT_AP_AUDIT_CHUTE_3_COINS     1723
#define SOUND_EFFECT_AP_AUDIT_TOTAL_REPLAYS     1724
#define SOUND_EFFECT_AP_AUDIT_AVG_BALL_TIME     1725
#define SOUND_EFFECT_AP_AUDIT_HISCR_BEAT        1726
#define SOUND_EFFECT_AP_AUDIT_TOTAL_BALLS       1727
#define SOUND_EFFECT_AP_AUDIT_NUM_MATCHES       1728
#define SOUND_EFFECT_AP_AUDIT_MATCH_PERCENTAGE  1729
#define SOUND_EFFECT_AP_AUDIT_LIFETIME_PLAYS    1730
#define SOUND_EFFECT_AP_AUDIT_MINUTES_ON        1731
#define SOUND_EFFECT_AP_AUDIT_CLEAR_AUDITS      1732

#define OM_BASIC_ADJ_IDS_FREEPLAY               0
#define OM_BASIC_ADJ_IDS_BALL_SAVE              1
#define OM_BASIC_ADJ_IDS_TILT_WARNINGS          2
#define OM_BASIC_ADJ_IDS_MUSIC_VOLUME           3
#define OM_BASIC_ADJ_IDS_SOUNDFX_VOLUME         4
#define OM_BASIC_ADJ_IDS_CALLOUTS_VOLUME        5
#define OM_BASIC_ADJ_IDS_BALLS_PER_GAME         6
#define OM_BASIC_ADJ_IDS_TOURNAMENT_MODE        7
#define OM_BASIC_ADJ_IDS_EXTRA_BALL_VALUE       8
#define OM_BASIC_ADJ_IDS_SPECIAL_VALUE          9 
#define OM_BASIC_ADJ_IDS_RESET_DURING_GAME      10
#define OM_BASIC_ADJ_IDS_SCORE_LEVEL_1          11
#define OM_BASIC_ADJ_IDS_SCORE_LEVEL_2          12
#define OM_BASIC_ADJ_IDS_SCORE_LEVEL_3          13
#define OM_BASIC_ADJ_IDS_SCORE_AWARDS           14
#define OM_BASIC_ADJ_IDS_SCROLLING_SCORES       15
#define OM_BASIC_ADJ_IDS_HISCR                  16
#define OM_BASIC_ADJ_IDS_CREDITS                17
#define OM_BASIC_ADJ_IDS_CPC_1                  18
#define OM_BASIC_ADJ_IDS_CPC_2                  19
#define OM_BASIC_ADJ_IDS_CPC_3                  20
#define OM_BASIC_ADJ_IDS_MATCH_FEATURE          21
#define OM_BASIC_ADJ_FINISHED                   22
#define SOUND_EFFECT_AP_FREEPLAY                (1740 + OM_BASIC_ADJ_IDS_FREEPLAY)
#define SOUND_EFFECT_AP_BALL_SAVE_SECONDS       (1740 + OM_BASIC_ADJ_IDS_BALL_SAVE)
#define SOUND_EFFECT_AP_TILT_WARNINGS           (1740 + OM_BASIC_ADJ_IDS_TILT_WARNINGS)
#define SOUND_EFFECT_AP_MUSIC_VOLUME            (1740 + OM_BASIC_ADJ_IDS_MUSIC_VOLUME)
#define SOUND_EFFECT_AP_SOUNDFX_VOLUME          (1740 + OM_BASIC_ADJ_IDS_SOUNDFX_VOLUME)
#define SOUND_EFFECT_AP_CALLOUTS_VOLUME         (1740 + OM_BASIC_ADJ_IDS_CALLOUTS_VOLUME)
#define SOUND_EFFECT_AP_BALLS_PER_GAME          (1740 + OM_BASIC_ADJ_IDS_BALLS_PER_GAME)
#define SOUND_EFFECT_AP_TOURNAMENT_MODE         (1740 + OM_BASIC_ADJ_IDS_TOURNAMENT_MODE)
#define SOUND_EFFECT_AP_EXTRA_BALL_VALUE        (1740 + OM_BASIC_ADJ_IDS_EXTRA_BALL_VALUE)
#define SOUND_EFFECT_AP_SPECIAL_VALUE           (1740 + OM_BASIC_ADJ_IDS_SPECIAL_VALUE)
#define SOUND_EFFECT_AP_RESET_DURING_GAME       (1740 + OM_BASIC_ADJ_IDS_RESET_DURING_GAME)
#define SOUND_EFFECT_AP_ADJ_SCORE_LEVEL_1       (1740 + OM_BASIC_ADJ_IDS_SCORE_LEVEL_1)
#define SOUND_EFFECT_AP_ADJ_SCORE_LEVEL_2       (1740 + OM_BASIC_ADJ_IDS_SCORE_LEVEL_2)
#define SOUND_EFFECT_AP_ADJ_SCORE_LEVEL_3       (1740 + OM_BASIC_ADJ_IDS_SCORE_LEVEL_3)
#define SOUND_EFFECT_AP_SCORE_AWARDS            (1740 + OM_BASIC_ADJ_IDS_SCORE_AWARDS)
#define SOUND_EFFECT_AP_SCROLLING_SCORES        (1740 + OM_BASIC_ADJ_SCROLLING_SCORES)
#define SOUND_EFFECT_AP_ADJ_HISCR               (1740 + OM_BASIC_ADJ_IDS_HISCR)
#define SOUND_EFFECT_AP_ADJ_CREDITS             (1740 + OM_BASIC_ADJ_IDS_CREDITS)
#define SOUND_EFFECT_AP_ADJ_CPC_1               (1740 + OM_BASIC_ADJ_IDS_CPC_1)
#define SOUND_EFFECT_AP_ADJ_CPC_2               (1740 + OM_BASIC_ADJ_IDS_CPC_2)
#define SOUND_EFFECT_AP_ADJ_CPC_3               (1740 + OM_BASIC_ADJ_IDS_CPC_3)
#define SOUND_EFFECT_AP_MATCH_FEATURE           (1740 + OM_BASIC_ADJ_IDS_MATCH_FEATURE)

#define SOUND_EFFECT_OM_EASY_RULES_INSTRUCTIONS           1770
#define SOUND_EFFECT_OM_MEDIUM_RULES_INSTRUCTIONS         1771
#define SOUND_EFFECT_OM_HARD_RULES_INSTRUCTIONS           1772
#define SOUND_EFFECT_OM_PROGRESSIVE_RULES_INSTRUCTIONS    1773
#define SOUND_EFFECT_OM_CUSTOM_RULES_INSTRUCTIONS         1774

#define OM_GAME_ADJ_EASY_DIFFICULTY                 0
#define OM_GAME_ADJ_MEDIUM_DIFFICULTY               1
#define OM_GAME_ADJ_HARD_DIFFICULTY                 2
#define OM_GAME_ADJ_PROGRESSIVE_DIFFICULTY          3
#define OM_GAME_ADJ_CUSTOM_DIFFICULTY               4
#define SOUND_EFFECT_AP_DIFFICULTY                  (1790 + OM_GAME_ADJ_EASY_DIFFICULTY)

#define OM_GAME_ADJ_TROUGH_EJECT_STRENGTH           0
#define OM_GAME_ADJ_SAUCER_EJECT_STRENGTH           1
#define OM_GAME_ADJ_SLINGSHOT_STRENGTH              2
#define OM_GAME_ADJ_POP_BUMPER_STRENGTH             3
#define OM_GAME_ADJ_MINIMODE_REQUALIFY_BEHAVIOR     4
#define OM_GAME_ADJ_FINISHED                        5
#define SOUND_EFFECT_AP_LOCK_BEHAVIOR               (1800 + OM_GAME_ADJ_TROUGH_EJECT_STRENGTH)


unsigned long SoundSettingTimeout;
unsigned long SoundTestStart;
unsigned short RestoreBackgroundTrack = BACKGROUND_TRACK_NONE;
byte SoundTestSequence;
boolean FirstTimeThroughOperatorMenu = true;
boolean FlippersDisabledLeavingOperatorMenu = false;

void RunOperatorMenu() {

  if (FirstTimeThroughOperatorMenu) {
    FirstTimeThroughOperatorMenu = false;
    FlippersDisabledLeavingOperatorMenu = RPU_GetDisableFlippers();
    if (FlippersDisabledLeavingOperatorMenu) RPU_SetDisableFlippers(false);
  }
  
  if (!Menus.UpdateMenu(CurrentTime)) {
    // Menu is done
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    Audio.StopAllAudio();
    RPU_TurnOffAllLamps();
    if (MachineState==MACHINE_STATE_ATTRACT) {
      RPU_SetDisplayBallInPlay(0, true);
    } else {
      RPU_SetDisplayBallInPlay(CurrentBallInPlay);
    }
    SoundSettingTimeout = 0;
    return;
  }

  // It's up to this function to eject balls if requested
  if (Menus.BallEjectInProgress()) {
    if (CountBallsInTrough()) {
      if (CurrentTime > (LastTimeBallServed+1500)) {
        LastTimeBallServed = CurrentTime;
        RPU_PushToSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, true);
      }
    }
  } else {
    LastTimeBallServed = 0;
  }
  
  byte topLevel = Menus.GetTopLevel();
  byte subLevel = Menus.GetSubLevel();

  if (Menus.HasTopLevelChanged()) {
    // Play an audio prompt for the top level
    SoundTestStart = 0;
    if (Audio.GetBackgroundSong()!=BACKGROUND_TRACK_NONE) {
      RestoreBackgroundTrack = Audio.GetBackgroundSong();
    }
    Audio.StopAllAudio();
    Audio.PlaySound((unsigned short)topLevel + SOUND_EFFECT_AP_TOP_LEVEL_MENU_ENTRY, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
    if (Menus.GetTopLevel()==OPERATOR_MENU_GAME_RULES_LEVEL) Menus.SetNumSubLevels(4);
    if (Menus.GetTopLevel()==OPERATOR_MENU_BASIC_ADJ_MENU) {
      GetCPCSelection(0); // make sure CPC values have been read
      Menus.SetNumSubLevels(OM_BASIC_ADJ_FINISHED);
    }
    if (Menus.GetTopLevel()==OPERATOR_MENU_GAME_ADJ_MENU) Menus.SetNumSubLevels(OM_GAME_ADJ_FINISHED);
  }
  if (Menus.HasSubLevelChanged()) {
    SoundTestStart = 0;
    // Play an audio prompt for the sub level    
    Audio.StopAllAudio();
    if (topLevel==OPERATOR_MENU_SELF_TEST_MENU) {
      Audio.PlaySound((unsigned short)subLevel + SOUND_EFFECT_AP_TEST_LAMPS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);

      if (subLevel==OPERATOR_MENU_TEST_SOUNDS) {
        SoundTestStart = CurrentTime + 1000;
        SoundTestSequence = 0;
      } else {
        SoundTestStart = 0;
      }
    } else if (topLevel==OPERATOR_MENU_AUDITS_MENU) {
      unsigned long *currentAdjustmentUL = NULL;
      byte currentAdjustmentStorageByte = 0;
      byte adjustmentType = OPERATOR_MENU_AUD_CLEARABLE;

      switch (subLevel) {
        case 0:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_TOTAL_PLAYS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_TOTAL_PLAYS_EEPROM_START_BYTE;
          break;
        case 1:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_CHUTE_1_COINS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_CHUTE_1_COINS_START_BYTE;
          break;
        case 2:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_CHUTE_2_COINS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_CHUTE_2_COINS_START_BYTE;
          break;
        case 3:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_CHUTE_3_COINS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_CHUTE_3_COINS_START_BYTE;
          break;
        case 4:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_TOTAL_REPLAYS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_TOTAL_REPLAYS_EEPROM_START_BYTE;
          break;
        case 5:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_HISCR_BEAT, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_TOTAL_HISCORE_BEATEN_START_BYTE;
          break;
      }

      Menus.SetAuditControls(currentAdjustmentUL, currentAdjustmentStorageByte, adjustmentType);

    } else if (topLevel==OPERATOR_MENU_BASIC_ADJ_MENU) {
      Audio.PlaySound((unsigned short)subLevel + SOUND_EFFECT_AP_FREEPLAY, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);

      byte *currentAdjustmentByte = NULL;
      byte currentAdjustmentStorageByte = 0;
      byte adjustmentValues[8] = {0};
      byte numAdjustmentValues = 2;
      byte adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
      short parameterCallout = 0;
      unsigned long *currentAdjustmentUL = NULL;
      
      adjustmentValues[1] = 1;

      switch(subLevel) {
        case OM_BASIC_ADJ_IDS_FREEPLAY:
          currentAdjustmentByte = (byte *)&FreePlayMode;
          currentAdjustmentStorageByte = EEPROM_FREE_PLAY_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_BALL_SAVE:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_LIST;
          numAdjustmentValues = 5;
          adjustmentValues[1] = 5;
          adjustmentValues[2] = 10;
          adjustmentValues[3] = 15;
          adjustmentValues[4] = 20;
          currentAdjustmentByte = &BallSaveNumSeconds;
          currentAdjustmentStorageByte = EEPROM_BALL_SAVE_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_TILT_WARNINGS:
          adjustmentValues[1] = 2;
          currentAdjustmentByte = &MaxTiltWarnings;
          currentAdjustmentStorageByte = EEPROM_TILT_WARNING_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_MUSIC_VOLUME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 10;
          currentAdjustmentByte = &MusicVolume;
          currentAdjustmentStorageByte = EEPROM_MUSIC_VOLUME_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SOUNDFX_VOLUME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 10;
          currentAdjustmentByte = &SoundEffectsVolume;
          currentAdjustmentStorageByte = EEPROM_SFX_VOLUME_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_CALLOUTS_VOLUME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 10;
          currentAdjustmentByte = &CalloutsVolume;
          currentAdjustmentStorageByte = EEPROM_CALLOUTS_VOLUME_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_BALLS_PER_GAME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          numAdjustmentValues = 8;
          adjustmentValues[0] = 3;
          adjustmentValues[1] = 10;
          currentAdjustmentByte = &BallsPerGame;
          currentAdjustmentStorageByte = EEPROM_BALLS_OVERRIDE_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_TOURNAMENT_MODE:
          currentAdjustmentByte = (byte *)&TournamentScoring;
          currentAdjustmentStorageByte = EEPROM_TOURNAMENT_SCORING_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_EXTRA_BALL_VALUE:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &ExtraBallValue;
          currentAdjustmentStorageByte = EEPROM_EXTRA_BALL_SCORE_UL;
          break;
        case OM_BASIC_ADJ_IDS_SPECIAL_VALUE:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &SpecialValue;
          currentAdjustmentStorageByte = EEPROM_SPECIAL_SCORE_UL;
          break;
        case OM_BASIC_ADJ_IDS_RESET_DURING_GAME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_LIST;
          numAdjustmentValues = 5;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 1;
          adjustmentValues[2] = 2;
          adjustmentValues[3] = 3;
          adjustmentValues[4] = 99;
          currentAdjustmentByte = &TimeRequiredToResetGame;
          currentAdjustmentStorageByte = EEPROM_CRB_HOLD_TIME;
          parameterCallout = SOUND_EFFECT_OM_CRB_VALUES;
          break;
        case OM_BASIC_ADJ_IDS_SCORE_LEVEL_1:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &AwardScores[0];
          currentAdjustmentStorageByte = RPU_AWARD_SCORE_1_EEPROM_START_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SCORE_LEVEL_2:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &AwardScores[1];
          currentAdjustmentStorageByte = RPU_AWARD_SCORE_2_EEPROM_START_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SCORE_LEVEL_3:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &AwardScores[2];
          currentAdjustmentStorageByte = RPU_AWARD_SCORE_3_EEPROM_START_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SCORE_AWARDS:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX_DEFAULT;
          adjustmentValues[1] = 7;
          currentAdjustmentByte = &ScoreAwardReplay;
          currentAdjustmentStorageByte = EEPROM_AWARD_OVERRIDE_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SCROLLING_SCORES:
          currentAdjustmentByte = (byte *)&ScrollingScores;
          currentAdjustmentStorageByte = EEPROM_SCROLLING_SCORES_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_HISCR:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &HighScore;
          currentAdjustmentStorageByte = RPU_HIGHSCORE_EEPROM_START_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_CREDITS:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 40;
          currentAdjustmentByte = &Credits;
          currentAdjustmentStorageByte = RPU_CREDITS_EEPROM_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_CPC_1:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_CPC;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = (NUM_CPC_PAIRS-1);
          currentAdjustmentByte = &(CPCSelection[0]);
          currentAdjustmentStorageByte = RPU_CPC_CHUTE_1_SELECTION_BYTE;
          parameterCallout = SOUND_EFFECT_OM_CPC_VALUES;
          break;
        case OM_BASIC_ADJ_IDS_CPC_2:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_CPC;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = (NUM_CPC_PAIRS-1);
          currentAdjustmentByte = &(CPCSelection[1]);
          currentAdjustmentStorageByte = RPU_CPC_CHUTE_2_SELECTION_BYTE;
          parameterCallout = SOUND_EFFECT_OM_CPC_VALUES;
          break;
        case OM_BASIC_ADJ_IDS_CPC_3:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_CPC;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = (NUM_CPC_PAIRS-1);
          currentAdjustmentByte = &(CPCSelection[2]);
          currentAdjustmentStorageByte = RPU_CPC_CHUTE_3_SELECTION_BYTE;
          parameterCallout = SOUND_EFFECT_OM_CPC_VALUES;
          break;
        case OM_BASIC_ADJ_IDS_MATCH_FEATURE:
          currentAdjustmentByte = (byte *)&MatchFeature;
          currentAdjustmentStorageByte = EEPROM_MATCH_FEATURE_BYTE;
          break;
      }

      Menus.SetParameterControls(   adjustmentType, numAdjustmentValues, adjustmentValues, parameterCallout,
                                    currentAdjustmentStorageByte, currentAdjustmentByte, currentAdjustmentUL );
    } else if (topLevel==OPERATOR_MENU_GAME_RULES_LEVEL) {
      Audio.PlaySound((unsigned short)subLevel + SOUND_EFFECT_AP_DIFFICULTY, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
      byte *currentAdjustmentByte = &GameRulesSelection;
      byte adjustmentValues[8] = {0};
      adjustmentValues[0] = 0;
      // if one of the below parameters is installed, the "HasParameterChanged" 
      // check below will install Easy / Medium / Hard rules

      switch (subLevel) {
        case 0:
          adjustmentValues[1] = 1;
          break;
        case 1:
          adjustmentValues[1] = 2;
          break;
        case 2:
          adjustmentValues[1] = 3;
          break;
        case 3:
          adjustmentValues[1] = 4;
          break;
      }

      Menus.SetParameterControls(   OPERATOR_MENU_ADJ_TYPE_LIST, 2, adjustmentValues, (short)SOUND_EFFECT_OM_EASY_RULES_INSTRUCTIONS-1,
                                    EEPROM_GAME_RULES_SELECTION, currentAdjustmentByte, NULL );
                  
    } else if (topLevel==OPERATOR_MENU_GAME_ADJ_MENU) {
      Audio.PlaySound((unsigned short)subLevel + SOUND_EFFECT_AP_LOCK_BEHAVIOR, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);

      byte *currentAdjustmentByte = NULL;
      byte currentAdjustmentStorageByte = 0;
      byte adjustmentValues[8] = {0};
      byte numAdjustmentValues = 2;
      byte adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
      short parameterCallout = 0;
      unsigned long *currentAdjustmentUL = NULL;
      
      adjustmentValues[1] = 1;

      switch (subLevel) {
        case OM_GAME_ADJ_TROUGH_EJECT_STRENGTH:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_LIST;
          numAdjustmentValues = 6;
          adjustmentValues[0] = 10;
          adjustmentValues[1] = 20;
          adjustmentValues[2] = 30;
          adjustmentValues[3] = 40;
          adjustmentValues[4] = 50;
          adjustmentValues[5] = 60;
          currentAdjustmentByte = &BallServeSolenoidStrength;
          currentAdjustmentStorageByte = EEPROM_TROUGH_EJECT_STRENGTH;
          break;
        case OM_GAME_ADJ_SAUCER_EJECT_STRENGTH:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 5;
          adjustmentValues[1] = 15;
          currentAdjustmentByte = &SaucerSolenoidStrength;
          currentAdjustmentStorageByte = EEPROM_SAUCER_EJECT_STRENGTH;
          break;
        case OM_GAME_ADJ_SLINGSHOT_STRENGTH:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 4;
          adjustmentValues[1] = 8;
//          currentAdjustmentByte = &TempSlingStrength;
          currentAdjustmentStorageByte = EEPROM_SLINGSHOT_STRENGTH;
          break;
        case OM_GAME_ADJ_POP_BUMPER_STRENGTH:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 4;
          adjustmentValues[1] = 8;
//          currentAdjustmentByte = &TempPopStrength;
          currentAdjustmentStorageByte = EEPROM_POP_BUMPER_STRENGTH;
          break;
      }
      
      Menus.SetParameterControls(   adjustmentType, numAdjustmentValues, adjustmentValues, parameterCallout,
                                    currentAdjustmentStorageByte, currentAdjustmentByte, currentAdjustmentUL );
    }    
  }

  if (Menus.HasParameterChanged()) {
    short parameterCallout = Menus.GetParameterCallout();
    if (parameterCallout) {
      Audio.StopAllAudio();
      Audio.PlaySound((unsigned short)parameterCallout + Menus.GetParameterID(), AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
    }
    if (Menus.GetTopLevel()==OPERATOR_MENU_GAME_RULES_LEVEL) {
      // Install the new rules level
      if (LoadRuleDefaults(GameRulesSelection)) {
        WriteParameters();
      }
    } else if (Menus.GetTopLevel()==OPERATOR_MENU_BASIC_ADJ_MENU) {
      if (Menus.GetSubLevel()==OM_BASIC_ADJ_IDS_MUSIC_VOLUME) {
        if (SoundSettingTimeout) Audio.StopAllAudio();
        Audio.PlaySound(SOUND_EFFECT_BACKGROUND_SONG_1, AUDIO_PLAY_TYPE_WAV_TRIGGER, MusicVolume);
        Audio.SetMusicVolume(MusicVolume);
        SoundSettingTimeout = CurrentTime + 5000;
      } else if (Menus.GetSubLevel()==OM_BASIC_ADJ_IDS_SOUNDFX_VOLUME) {
        if (SoundSettingTimeout) Audio.StopAllAudio();
        Audio.PlaySound(SOUND_EFFECT_SPINNER_UNLIT, AUDIO_PLAY_TYPE_WAV_TRIGGER, SoundEffectsVolume);
        Audio.SetSoundFXVolume(SoundEffectsVolume);
        SoundSettingTimeout = CurrentTime + 5000;
      } else if (Menus.GetSubLevel()==OM_BASIC_ADJ_IDS_CALLOUTS_VOLUME) {
        if (SoundSettingTimeout) Audio.StopAllAudio();
        Audio.PlaySound(SOUND_EFFECT_VP_SHOOT_AGAIN, AUDIO_PLAY_TYPE_WAV_TRIGGER, CalloutsVolume);
        Audio.SetNotificationsVolume(CalloutsVolume);
        SoundSettingTimeout = CurrentTime + 3000;
      }
    }
  }

  if (SoundSettingTimeout && CurrentTime>SoundSettingTimeout) {
    SoundSettingTimeout = 0;
    Audio.StopAllAudio();
  }
  
  if (!Menus.OperatorMenusActive()) {
    RPU_SetDisableFlippers(FlippersDisabledLeavingOperatorMenu);
  }
}



////////////////////////////////////////////////////////////////////////////
//
//  Audio Output functions
//
////////////////////////////////////////////////////////////////////////////
void PlayBackgroundSong(unsigned int songNum) {
  if (MusicVolume == 0) return;
  Audio.PlayBackgroundSong(songNum);
}


unsigned long NextSoundEffectTime = 0;

/*
 * Original Sounds
 *  1 - Tilt
 *  2 - ?
 *  3 - lock
 *  4 - player 2 sound
 *  5 - slingshot
 *  6 - 
 *  7 - *you are destroyed
 *  8 - *you won one mission
 *  9 - top lane
 *  10 - pop
 *  11 - bonus
 *  12 - *mission accomplished
 *  13 - *Fire
 *  14 - *enemy destroyed
 *  15 - *firepower mission accomplished
 *  16 - 
 *  17 - background
 *  18 - repeating something
 *  19 - stop background
 *  20 - launch explosion
 *  21 - ready background
 *  22 - 
 *  23 - short explosion
 *  24 - *fire 1
 *  25 - finish bank
 *  26 - *power
 *  27 - *you destroyed enemy mission
 *  28 - match
 *  29 - defender something
 *  30 - *firepower
 *  31 - *firepower
 *  32 - 
 *  
 */


void PlaySoundEffect(unsigned int soundEffectNum) {

  if (MachineState == MACHINE_STATE_INIT_GAMEPLAY) return;

  // Play digital samples on the WAV trigger (numbered same
  // as SOUND_EFFECT_ defines)
  Audio.PlaySound(soundEffectNum, AUDIO_PLAY_TYPE_WAV_TRIGGER);
}


boolean AdvanceNickname(byte playerNumber) {
  byte basePlayerNum = 0;
  byte numNamesPerRank = NICKNAMES_LEVEL_1_QTY;
  byte newNameID = 0;

  // we only increase rank if the level is less than 4  
  for (byte count=0; count<4; count++) {
    if (count==1) numNamesPerRank = NICKNAMES_LEVEL_2_QTY;
    else if (count==2) numNamesPerRank = NICKNAMES_LEVEL_3_QTY;
    else if (count==3) numNamesPerRank = NICKNAMES_LEVEL_4_QTY;
    
    if (playerNumber<(basePlayerNum + numNamesPerRank)) {
      // count is currently the rank we're at, 
      // so we can increase by one
      byte numRanksAtNextLevel = NICKNAMES_LEVEL_2_QTY;
      if (count==1) numRanksAtNextLevel = NICKNAMES_LEVEL_3_QTY;
      if (count==2) numRanksAtNextLevel = NICKNAMES_LEVEL_4_QTY;
      if (count==3) numRanksAtNextLevel = NICKNAMES_LEVEL_5_QTY;

      // find a random name at the next level
      newNameID = (basePlayerNum + numNamesPerRank) + (CurrentTime%numRanksAtNextLevel);
      PlayerNickname[playerNumber] = newNameID;
      return true;
    }
  }
  return false;
}


int GetRankSoundIndex(byte rankNumber) {
  return SOUND_EFFECT_VP_RANK_1 + rankNumber;
}

int GetNicknameSoundIndex(byte nicknameNumber, boolean primaryNickname) {
  int nicknameSoundID = NICKNAMES_LEVEL_1_START + nicknameNumber*3;

  if (primaryNickname) return nicknameSoundID;
  return nicknameSoundID + ((CurrentTime/4)%3);
}


void QueueNotification(unsigned int soundEffectNum, byte priority) {
  if (CalloutsVolume == 0) return;

  // With RPU_OS_HARDWARE_REV 4 and above, the WAV trigger has two-way communication,
  // so it's not necesary to tell it the length of a notification. For support for
  // earlier hardware, you'll need an array of VoicePromptLengths for each prompt
  // played (for queueing and ducking)
  //  Audio.QueuePrioritizedNotification(soundEffectNum, VoicePromptLengths[soundEffectNum-SOUND_EFFECT_VP_VOICE_NOTIFICATIONS_START], priority, CurrentTime);
  Audio.QueuePrioritizedNotification(soundEffectNum, 0, priority, CurrentTime);

}


byte PlayerUpPhase = 0;
void AlertPlayerUp() {
  
  if ((PlayerUpPhase%4)==0) QueueNotification(SOUND_EFFECT_VP_PLAYER_1_UP + CurrentPlayer, 1);
  else if ((PlayerUpPhase%4)==1) QueueNotification(SOUND_EFFECT_VP_LETS_GO, 1);
  else if ((PlayerUpPhase%4)==2) QueueNotification(SOUND_EFFECT_VP_GET_OFF_THE_GROUND + CurrentPlayer, 1);
  else if ((PlayerUpPhase%4)==3) QueueNotification(SOUND_EFFECT_VP_BATTLE_AWAITS + CurrentPlayer, 1);

  if (PlayerUpPhase && (CurrentTime%2)) {
    QueueNotification(GetNicknameSoundIndex(PlayerNickname[CurrentPlayer], true), 1);
  }

  PlayerUpPhase += 1;
}




////////////////////////////////////////////////////////////////////////////
//
//  Diagnostics Mode
//
////////////////////////////////////////////////////////////////////////////

int RunDiagnosticsMode(int curState, boolean curStateChanged) {

  int returnState = curState;

  if (curStateChanged) {

    /*
        char buf[256];
        boolean errorSeen;

        Serial.write("Testing Volatile RAM at IC13 (0x0000 - 0x0080): writing & reading... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        errorSeen = false;
        for (byte valueCount=0; valueCount<0xFF; valueCount++) {
          for (unsigned short address=0x0000; address<0x0080; address++) {
            RPU_DataWrite(address, valueCount);
          }
          for (unsigned short address=0x0000; address<0x0080; address++) {
            byte readValue = RPU_DataRead(address);
            if (readValue!=valueCount) {
              sprintf(buf, "Write/Read failure at address=0x%04X (expected 0x%02X, read 0x%02X)\n", address, valueCount, readValue);
              Serial.write(buf);
              errorSeen = true;
            }
            if (errorSeen) break;
          }
          if (errorSeen) break;
        }
        if (errorSeen) {
          Serial.write("!!! Error in Volatile RAM\n");
        }

        Serial.write("Testing Volatile RAM at IC16 (0x0080 - 0x0100): writing & reading... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        errorSeen = false;
        for (byte valueCount=0; valueCount<0xFF; valueCount++) {
          for (unsigned short address=0x0080; address<0x0100; address++) {
            RPU_DataWrite(address, valueCount);
          }
          for (unsigned short address=0x0080; address<0x0100; address++) {
            byte readValue = RPU_DataRead(address);
            if (readValue!=valueCount) {
              sprintf(buf, "Write/Read failure at address=0x%04X (expected 0x%02X, read 0x%02X)\n", address, valueCount, readValue);
              Serial.write(buf);
              errorSeen = true;
            }
            if (errorSeen) break;
          }
          if (errorSeen) break;
        }
        if (errorSeen) {
          Serial.write("!!! Error in Volatile RAM\n");
        }

        // Check the CMOS RAM to see if it's operating correctly
        errorSeen = false;
        Serial.write("Testing CMOS RAM: writing & reading... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        for (byte valueCount=0; valueCount<0x10; valueCount++) {
          for (unsigned short address=0x0100; address<0x0200; address++) {
            RPU_DataWrite(address, valueCount);
          }
          for (unsigned short address=0x0100; address<0x0200; address++) {
            byte readValue = RPU_DataRead(address);
            if ((readValue&0x0F)!=valueCount) {
              sprintf(buf, "Write/Read failure at address=0x%04X (expected 0x%02X, read 0x%02X)\n", address, valueCount, (readValue&0x0F));
              Serial.write(buf);
              errorSeen = true;
            }
            if (errorSeen) break;
          }
          if (errorSeen) break;
        }

        if (errorSeen) {
          Serial.write("!!! Error in CMOS RAM\n");
        }


        // Check the ROMs
        Serial.write("CMOS RAM dump... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        for (unsigned short address=0x0100; address<0x0200; address++) {
          if ((address&0x000F)==0x0000) {
            sprintf(buf, "0x%04X:  ", address);
            Serial.write(buf);
          }
      //      RPU_DataWrite(address, address&0xFF);
          sprintf(buf, "0x%02X ", RPU_DataRead(address));
          Serial.write(buf);
          if ((address&0x000F)==0x000F) {
            Serial.write("\n");
          }
        }

    */

    //    RPU_EnableSolenoidStack();
    //    RPU_SetDisableFlippers(false);

  }

  return returnState;
}




////////////////////////////////////////////////////////////////////////////
//
//  Attract Mode
//
////////////////////////////////////////////////////////////////////////////
byte AttractLastMode = 255;
boolean AttractCheckedForTrappedBall;
unsigned long AttractModeStartTime;

int RunAttractMode(int curState, boolean curStateChanged) {

  int returnState = curState;

  if (curStateChanged) {
    // Some sound cards have a special index
    // for a "sound" that will turn off
    // the current background drone or currently
    // playing sound
    RPU_DisableSolenoidStack();
    RPU_TurnOffAllLamps();
    RPU_SetDisableFlippers(true);
    if (DEBUG_MESSAGES) {
      Serial.write("Entering Attract Mode\n\r");
    }
    AttractLastMode = 0;
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    Display_ClearOverride(0xFF);
    Display_UpdateDisplays(0xFF);
    AttractCheckedForTrappedBall = false;
    AttractModeStartTime = CurrentTime;
    Audio.PlaySoundCardWhenPossible(19 * 256, CurrentTime+4950, 0, 500, 10);
    ShowHeadAndApronLamps();
    if (!FreePlayMode) RPU_SetCoinLockout((Credits >= MaximumCredits) ? true : false, SOLCONT_COIN_LOCKOUT);
    else RPU_SetCoinLockout(false, SOLCONT_COIN_LOCKOUT);
  }

  EjectAllBallsFromSaucers();
  MoveBallFromOutholeToRamp();

  if (CurrentTime > (AttractModeStartTime + 5000) && !AttractCheckedForTrappedBall) {
    AttractCheckedForTrappedBall = true;
    if (DEBUG_MESSAGES) {
      Serial.write("In Attract for 10 seconds - make sure there are no balls trapped\n");
    }

    for (byte count=0; count<3; count++) {
      if (RPU_ReadSingleSwitchState(SaucerSwitches[count])) {
        RPU_PushToSolenoidStack(SaucerSolenoids[count], SaucerSolenoidStrength, true);
      }
    }
  }

  //RPU_SetLampState(LAMP_APRON_CREDITS, (FreePlayMode || Credits) ? true : false, 0, 200);

  if (CurrentTime<5000) {
    // If this is the first 5 seconds that the machine is on, 
    // we'll show version information
    if (AttractLastMode==0) {
      AttractLastMode = 1;
      Display_EnableAchievements(false);
      Display_OverrideScoreDisplay(0, GAME_MAJOR_VERSION, DISPLAY_OVERRIDE_ANIMATION_CENTER);
      Display_OverrideScoreDisplay(1, GAME_MINOR_VERSION, DISPLAY_OVERRIDE_ANIMATION_CENTER);
      Display_OverrideScoreDisplay(2, RPU_OS_MAJOR_VERSION, DISPLAY_OVERRIDE_ANIMATION_CENTER);
      Display_OverrideScoreDisplay(3, RPU_OS_MINOR_VERSION, DISPLAY_OVERRIDE_ANIMATION_CENTER);
    }
    RPU_FlashAllLamps(CurrentTime);
    Display_UpdateDisplays(0xFF);
  } else {
    // Normal Attract Mode
    if (AttractLastMode<2) {
      if (AttractLastMode==1) {
        AttractModeStartTime = CurrentTime;
      }
      AttractLastMode = 2;
      Display_EnableAchievements(true);
      Display_ClearOverride(0xFF);
    }

    byte attractModePhase = ((CurrentTime - AttractModeStartTime)/4000) % 2;
    if (attractModePhase==0) {
      // Show high scores
      Display_UpdateDisplays(0xFF, false, false, false, HighScore);
      RPU_SetLampState(LAMP_HEAD_HIGH_SCORE, 1, 0, 125);
      ShowHeadAndApronLamps();
    } else {
      // Show last scores
      Display_UpdateDisplays(0xFF);
      RPU_SetLampState(LAMP_HEAD_HIGH_SCORE, 0);
      ShowHeadAndApronLamps();
    }
    RPU_SetLampState(LAMP_HEAD_MATCH, 0);

    byte attractPlayfieldPhase = (((CurrentTime - AttractModeStartTime) / 5000) % 6);
  
    if ( (attractPlayfieldPhase % 3)==0) {
      ShowLampAnimation(attractPlayfieldPhase % 3, 40, CurrentTime, LAMP_ANIMATION_STEPS-1, false, false);
    } else {
      ShowLampAnimation(attractPlayfieldPhase % 3, 20, CurrentTime, 18, false, false);
    }
    
  }

  byte switchHit;
  while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
    if (switchHit == SW_CREDIT_RESET) {
      if ((CurrentTime-LastStartButtonSwitchTime)>250) {
        LastStartButtonSwitchTime = CurrentTime;
        if (AddPlayer(true)) returnState = MACHINE_STATE_INIT_GAMEPLAY;
      }
    }
    if (switchHit == SW_COIN_1 || switchHit == SW_COIN_2 || switchHit == SW_COIN_3) {
      byte chuteNum = SwitchToChuteNum(switchHit);
      if ((CurrentTime-LastCoinSwitchTime[chuteNum])>250) {
        LastCoinSwitchTime[chuteNum] = CurrentTime;
        AddCoinToAudit(chuteNum);
        AddCoin(chuteNum);
      }
    }
    if (switchHit == SW_SELF_TEST_SWITCH) {
      Menus.EnterOperatorMenu();
    }
  }

  // If the user was holding the menu button when the game started
  // then kick the balls
  if (0 && CurrentTime < 4000) {    
    if (RPU_ReadSingleSwitchState(SW_SELF_TEST_SWITCH)) {
      if (OperatorSwitchPressStarted==0) {
        OperatorSwitchPressStarted = CurrentTime;
      } else if (CurrentTime > (OperatorSwitchPressStarted+500)) {
        Menus.EnterOperatorMenu();
        Menus.BallEjectInProgress(true);
      }
    } else {
      OperatorSwitchPressStarted = 0;
    }
  }

  return returnState;
}





////////////////////////////////////////////////////////////////////////////
//
//  Game Play functions
//
////////////////////////////////////////////////////////////////////////////
byte CountBits(unsigned short intToBeCounted) {
  byte numBits = 0;

  for (byte count = 0; count < 16; count++) {
    numBits += (intToBeCounted & 0x01);
    intToBeCounted = intToBeCounted >> 1;
  }

  return numBits;
}


void SetGameMode(byte newGameMode) {
  LastGameMode = GameMode;
  GameMode = newGameMode;
  GameModeStartTime = 0;
  GameModeEndTime = 0;

  if (DEBUG_MESSAGES) {
    char buf[128];
    sprintf(buf, "Game Mode = %d\n", newGameMode);
    Serial.write(buf);
  }
}

byte CountBallsInTrough() {

  byte numBalls = 0;

  numBalls += RPU_ReadSingleSwitchState(SW_TROUGH_1);
  numBalls += RPU_ReadSingleSwitchState(SW_TROUGH_2);
  numBalls += RPU_ReadSingleSwitchState(SW_TROUGH_3);

  return numBalls;
}



void AddToBonus(byte bonus) {
  Bonus[CurrentPlayer] += bonus;
  if (Bonus[CurrentPlayer] > MAX_DISPLAY_BONUS) {
    Bonus[CurrentPlayer] = MAX_DISPLAY_BONUS;
    if (!SpecialCollected) {
      if (SpecialAvailable) {
        AwardSpecial();
        SpecialAvailable = false;
      } else {
        SpecialAvailable = true;
      }
    }
  } else {
    BonusChanged = CurrentTime;
  }
}



boolean IncreaseBonusX() {
  if (BonusX[CurrentPlayer] < 9) {
    BonusX[CurrentPlayer] += 1;
    BonusXAnimationStart = CurrentTime;
    return true;
  }

  return false;
}



unsigned long GameStartNotificationTime = 0;
boolean WaitForBallToReachOuthole = false;
unsigned long UpperBallEjectTime = 0;

int InitGamePlay(boolean curStateChanged) {

  if (curStateChanged) {
    RPU_TurnOffAllLamps();
    Display_EnableAchievements(true);
    SetGeneralIlluminationOn(true);
    GameStartNotificationTime = CurrentTime;
    Audio.StopAllAudio();
    //QueueNotification(SOUND_EFFECT_VP_ADD_PLAYER_1, 10);
    PlaySoundEffect(SOUND_EFFECT_VP_ADD_PLAYER_1);
    for (byte count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) RPU_SetDisplayBlank(count, 0x00);
    RPU_SetDisplayCredits(0, false);
    RPU_SetDisplayBallInPlay(1, true);
    NumberOfBallsLocked = 0;

    boolean showBIP = (CurrentTime / 100) % 2;
    RPU_SetDisplayBallInPlay(1, showBIP ? true : false);
    
    // The start button has been hit only once to get
    // us into this mode, so we assume a 1-player game
    // at the moment
    if (!FreePlayMode) RPU_SetCoinLockout((Credits >= MaximumCredits) ? true : false, SOLCONT_COIN_LOCKOUT);
  
    // Reset displays & game state variables
    for (int count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
      // Initialize game-specific variables
      Bonus[count] = 0;
      BonusX[count] = 1;
      CurrentAchievements[count] = 0;
      CurrentScores[count] = 0;
      ExtraBallAvailable[count] = 0;
      PlayerLocks[count] = 0;
      StandupTargetStatus[count] = 0;
      StandupTargetCompletions[count] = 0;
      FireStatus[count] = 0;
      FireCompletions[count] = 0;
      PowerStatus[count] = 0;
      PowerCompletions[count] = 0;
      FirePowerLevel[count] = 0;
      PopHits[count] = 0;
      SpinnerHits[count] = 0;
      BattlesWon[count] = 0;
      JackpotValue[count] = 0;
      SkillShotType[count] = SKILL_SHOT_TYPE_ALWAYS_CHANGEABLE_LANE;
      SkillShotsHit[count] = 0;
      ShipWeapons[count] = 0;
      ShipThrusters[count] = 0;
      PlayerRank[count] = 0;
      PlayerTrainingStatus[count] = 0;
      if (BallKickerBehavior==BALL_KICKER_EASY) LeftKickback[count] = 1;
      else LeftKickback[count] = 0;
      BattlesPlayed[count] = 0;
      WeaponsTrainingHits[count] = 0;
      FlightTrainingHits[count] = 0;
      PopExtraValueInThousands[count] = 0;
      SpinnerExtraValueInHundreds[count] = 0;
      BossBattleShotThreshold[count] = 5;
    }

    SamePlayerShootsAgain = false;
    CurrentBallInPlay = 1;
    CurrentNumPlayers = 1;
    CurrentPlayer = 0;
    NumberOfBallsInPlay = 0;
    for (byte count=0; count<4; count++) {
      LastTimePopHit[count] = 0;
    }
    for (byte count=0; count<6; count++) {
      LastTimeStandupSeen[count] = 0;
    }
    LastTiltWarningTime = 0;
    Display_ClearOverride(0xFF);
    Display_UpdateDisplays(0xFF);
    RPU_EnableSolenoidStack();
    
    LastTimePromptPlayed = CurrentTime;
  }


  EjectAllBallsFromSaucers();
  MoveBallFromOutholeToRamp();

  if ( (CountBallsInTrough() + RPU_ReadSingleSwitchState(SW_SHOOTER_LANE))==TotalBallsLoaded ) {
    for (byte count=0; count<3; count++) SaucerClosedStart[count] = 0;
    // All the balls are back in their home so we can continue
    LastTimePromptPlayed = 0;
    return MACHINE_STATE_INIT_NEW_BALL;
  }

  // Every 10 seconds, announce that a ball is missing
  if (CurrentTime > (LastTimePromptPlayed+10000)) {
    QueueNotification(SOUND_EFFECT_VP_BALL_MISSING, 10);
    LastTimePromptPlayed = CurrentTime;
  }

  boolean showBIP = (CurrentTime / 100) % 2;
  RPU_SetDisplayBallInPlay(1, showBIP ? true : false);

  return MACHINE_STATE_INIT_GAMEPLAY;
}



int InitNewBall(bool curStateChanged) {

  // If we're coming into this mode for the first time
  // then we have to do everything to set up the new ball
  if (curStateChanged) {
    //RPU_FireContinuousSolenoid(0x20, 5);
    RPU_TurnOffAllLamps();
    BallFirstSwitchHitTime = 0;

    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    if (CurrentNumPlayers > 1 && (CurrentBallInPlay != 1 || CurrentPlayer != 0) && !SamePlayerShootsAgain) AlertPlayerUp();
    SamePlayerShootsAgain = false;

    RPU_SetDisplayBallInPlay(CurrentBallInPlay);
    for (byte count = 0; count < 4; count++) {
      if (count==CurrentPlayer) RPU_SetLampState(PlayerUpLamps[count], 1, 0, 250);
      else if (count<CurrentNumPlayers) RPU_SetLampState(PlayerUpLamps[count], 1);
      else RPU_SetLampState(PlayerUpLamps[count], 0);
      RPU_SetDisplayBlank(count, 0);
    }

    if (BallSaveNumSeconds > 0) {
      RPU_SetLampState(LAMP_SHOOT_AGAIN, 1, 0, 500);
    }

    NumTiltWarnings = 0;
    BallSaveUsed = false;
    BallSaveNumSeconds = BallSaveNewBall;
    BallTimeInTrough = 0;
    BallKickerEndTime = 0;

    // Initialize game-specific start-of-ball lights & variables
    GameModeStartTime = 0;
    GameModeEndTime = 0;
    GameMode = GAME_MODE_SKILL_SHOT;
    PlayerUpPhase = 0;

    if (ResetShipWeaponsEachBall) {
      ShipWeapons[CurrentPlayer] = 0;
      ShipThrusters[CurrentPlayer] = 0;
    }
    
    SpecialCollected = false;
    SpecialAvailable = false;
    JackpotReady = false;

    if (!HoldoverTraining) {
      if (!WeaponsTrainingHolds) PopHits[CurrentPlayer] = 0;
      if (!FlightTrainingHolds) SpinnerHits[CurrentPlayer] = 0;
    }
    
    if (PlayerTrainingStatus[CurrentPlayer]&WEAPONS_TRAINING_RUNNING) {
      // The last ball must have ended during training
      // so we can put it back to qualified
      PlayerTrainingStatus[CurrentPlayer] &= ~WEAPONS_TRAINING_RUNNING;
      PlayerTrainingStatus[CurrentPlayer] |= WEAPONS_TRAINING_QUALIFIED;
    }
    if (PlayerTrainingStatus[CurrentPlayer]&FLIGHT_TRAINING_RUNNING) {
      // The last ball must have ended during training
      // so we can put it back to qualified
      PlayerTrainingStatus[CurrentPlayer] &= ~FLIGHT_TRAINING_RUNNING;
      PlayerTrainingStatus[CurrentPlayer] |= FLIGHT_TRAINING_QUALIFIED;
    }

    WeaponsTrainingEndTime = 0;
    FlightTrainingEndTime = 0;
    OfferBattleSaucer = 0xFF;
    SecondBattleSaucer = 0xFF;
    ThirdBattleSaucer = 0xFF;
    LastTimeBattleShotHit = 0;
    LastTimeLockShotHit = 0;
    LastTimeShipUpgraded = 0;
    LastTimeTrainingCalloutMade = 0;
    LastTimeWeaponsTrainingHit = 0;
    LastTimeFlightTrainingHit = 0;
    LastTrainingHitTime = 0;    
    LeftInlaneLastHitTime = 0;
    RightInlaneLastHitTime = 0;
    LeftInlaneStage = 0;
    RightInlaneStage = 0;
    PowerComboCollected = false;

    BossBattleStage = 0;
    BossBattleTimer = 0;
    BossBattleFuelBurn = false;
    BossBattleBallInShooter = 0;
    BossBattlePlungeGrace = 3;
    BossBattleStagesDone = 0;
    BossBattleBonus = 0;
    for (byte count=0; count<3; count++) BossSaucerKickoutTime[count] = 0;

    PlayfieldMultiplier = 1;
    PlayfieldMultiplierExpirationTime = 0;
    BonusXAnimationStart = 0;
    Bonus[CurrentPlayer] = 1;
    BonusX[CurrentPlayer] = 1;
    Display_ResetDisplayTrackingVariables();
    for (byte count=0; count<4; count++) {
      LastTimePopHit[count] = 0;
      FireLaneHitTime[count] = 0;
    }
    for (byte count=0; count<6; count++) {
      LastTimeStandupSeen[count] = 0;
    }

    for (byte count=0; count<3; count++) {
      PowerTargetHitTime[count] = 0;
    }

    SetBallSave(0);

    FireCompletedTime = 0;
    PowerCompletedTime = 0;
    StandupTargetFinishTime = 0;
    FireLevelChangedTime = 0;
    PowerLevelChangedTime = 0;
    FirePowerLevelChangedTime = 0;
    for (byte count=0; count<3; count++) {
      LastTimeSaucerSeen[count] = 0;
    }

    // if we have any player locks that have been taken,
    // or player locks qualified that have been filled,
    // we have to adjust our expectations
    UpdatePlayerLocks();

    if (RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)==0) RPU_PushToTimedSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, CurrentTime + 1000, true);
    LastTimeBallServed = CurrentTime + 1000;
    
    NumberOfBallsInPlay = 1;

//    Audio.OutputTracksPlaying();
    short curBackgroundSong = SOUND_EFFECT_BACKGROUND_SONG_6;
    if (CurrentBallInPlay>1 && CurrentBallInPlay<BallsPerGame) {
      curBackgroundSong = SOUND_EFFECT_BACKGROUND_SONG_1 + 1 + (CurrentTime%3);
    } else if (CurrentBallInPlay==BallsPerGame) {
      curBackgroundSong = SOUND_EFFECT_BACKGROUND_SONG_5;
    }
    PlayBackgroundSong(curBackgroundSong);
//    Audio.OutputTracksPlaying();
    RPU_EnableSolenoidStack();
    RPU_SetDisableFlippers(false);
    ShowHeadAndApronLamps();
  }

  MoveBallFromOutholeToRamp();
  LastTimeThroughLoop = CurrentTime;

  // Now we'll wait for the ball to get to the shooter lane switch
  if (RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) {
    return MACHINE_STATE_NORMAL_GAMEPLAY;
  }

  // if the ball hasn't gotten to the shooter lane, try to kick
  // it again periodically, but only if the number of balls
  // in the trough plus the number of locked balls makes sense
  if ( (CountBallsInTrough()+NumberOfBallsLocked)==TotalBallsLoaded ) {
    if ( CurrentTime > (LastTimeBallServed+3000) ) {
      // only serve a ball every 3 seconds
      RPU_PushToTimedSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, CurrentTime, true);
      LastTimeBallServed = CurrentTime;
    }
  }

  return MACHINE_STATE_INIT_NEW_BALL;
}




void CheckSaucersForBall() {

  for (byte count=0; count<3; count++) {
    if (RPU_ReadSingleSwitchState(SaucerSwitches[count])) {
      if (SaucerClosedStart[count]==0) {
        if (DEBUG_MESSAGES) Serial.write("Saw saucer switch\n");
        SaucerClosedStart[count] = CurrentTime;
      } else if (SaucerClosedStart[count]!=1) {
        if (CurrentTime > (SaucerClosedStart[count] + 250)) {
          if (DEBUG_MESSAGES) Serial.write("Handling saucer\n");
          HandleSaucer(count);
          SaucerClosedStart[count] = 1;
        }
      }
    } else {
      SaucerClosedStart[count] = 0;
    }
  }

  // Here we need to check to see if
  // a ball was knocked out of a saucer
  // and go to breach multiball

}



void UpdateTimers() {
  if (FireCompletedTime && CurrentTime>(FireCompletedTime+2000)) {
    FireCompletedTime = 0;
  }
  
  if (PowerCompletedTime && CurrentTime>(PowerCompletedTime+2000)) {
    PowerCompletedTime = 0;
  }
  
  if (StandupTargetFinishTime && CurrentTime>(StandupTargetFinishTime+2000)) {
    StandupTargetFinishTime = 0;
  }

  if (FireLevelChangedTime && CurrentTime>(FireLevelChangedTime+3000)) {
    FireLevelChangedTime = 0;
  }
  
  if (PowerLevelChangedTime && CurrentTime>(PowerLevelChangedTime+3000)) {
    PowerLevelChangedTime = 0;
  }
  
  if (FirePowerLevelChangedTime && CurrentTime>(FirePowerLevelChangedTime+7500)) {
    FirePowerLevelChangedTime = 0;
  }

  for (byte count=0; count<3; count++) {
    if (PowerTargetHitTime[count] && CurrentTime>(PowerTargetHitTime[count]+1500)) {
      PowerTargetHitTime[count] = 0;
    }
  }

  if (PlayfieldMultiplierExpirationTime && CurrentTime>PlayfieldMultiplierExpirationTime) {
    PlayfieldMultiplier = 1;
    PlayfieldMultiplierExpirationTime = 0;
  }

  if (BallKickerEndTime && CurrentTime>BallKickerEndTime) {
    BallKickerEndTime = 0;
    LeftKickback[CurrentPlayer] -= 1;
  }

  if (LeftInlaneLastHitTime && CurrentTime>(LeftInlaneLastHitTime+((unsigned long)ComboDuration * 1000))) {
    LeftInlaneStage = 0;
    LeftInlaneLastHitTime = 0;
  }

  if (RightInlaneLastHitTime && CurrentTime>(RightInlaneLastHitTime+((unsigned long)ComboDuration * 1000))) {
    RightInlaneStage = 0;
    RightInlaneLastHitTime = 0;
  }

  for (byte count=0; count<4; count++) {
    if (FireLaneHitTime[count] && CurrentTime>(FireLaneHitTime[count]+1500)) {
      FireLaneHitTime[count] = 0;
    }
  }


  if (WeaponsTrainingEndTime && CurrentTime>WeaponsTrainingEndTime) {
    // turn off weapons training
    WeaponsTrainingEndTime = 0;

    if (WeaponsTrainingHits[CurrentPlayer]!=WeaponsTrainingStartingHits) {
      CompletePlayerTraining(WEAPONS_TRAINING);
    }    
  }

  if (FlightTrainingEndTime && CurrentTime>FlightTrainingEndTime) {
    // turn off flight training
    FlightTrainingEndTime = 0;
    PlayerTrainingStatus[CurrentPlayer] &= ~FLIGHT_TRAINING_RUNNING;

    if (FlightTrainingHits[CurrentPlayer]!=FlightTrainingStartingHits) {
      CompletePlayerTraining(FLIGHT_TRAINING);
    }    
  }

  if (GameMode==GAME_MODE_BOSS_BATTLE && RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) {
    if (BossBattleBallInShooter==0) {
      BossBattleBallInShooter = CurrentTime;
    } else if (CurrentTime>(BossBattleBallInShooter + ((unsigned long)BossBattlePlungeGrace*1000))) {
      BossBattleFuelBurn = true;
    }
  } else {
    BossBattleBallInShooter = 0;
    BossBattleFuelBurn = false;
  }

}



void QueueTrainingCalloutIfNecessary() {
/*
  // we'll only consider making a callout every 15 seconds
  if (CurrentTime < (LastTimeTrainingCalloutMade+15000)) return;

  // figure out which type of callout is warranted
  if (PlayerTrainingStatus[CurrentPlayer]&TRAINING_RUNNING_MASK) {
    // we have a training running, so we can prompt based on that
    byte trainingPriority = 0xFF;
    unsigned short dockingPercentage = 100;
    unsigned short weaponsPercentage = 100;
    unsigned short flightPercentage = 100;
    unsigned short navigationPercentage = 100;

    if (PlayerTrainingStatus[CurrentPlayer]&DOCKING_TRAINING_RUNNING) {
      dockingPercentage = ((int)DockingHits[CurrentPlayer]*100)/DockingTrainingGoal;
    }
    if (PlayerTrainingStatus[CurrentPlayer]&WEAPONS_TRAINING_RUNNING) {
      weaponsPercentage = ((int)PopHits[CurrentPlayer]*100)/WeaponsTrainingGoal;
    }
    if (PlayerTrainingStatus[CurrentPlayer]&FLIGHT_TRAINING_RUNNING) {
      flightPercentage = ((int)SpinnerHits[CurrentPlayer]*100)/FlightTrainingGoal;
    }    
    if (PlayerTrainingStatus[CurrentPlayer]&NAVIGATION_TRAINING_RUNNING) {
      int numSwitchesSeen = CountBits(TenPointSwitchHits[CurrentPlayer]);
      navigationPercentage = ((int)numSwitchesSeen*100)/NavigationTrainingGoal;
    }
    byte lowestPercentage = dockingPercentage; 
    if (dockingPercentage<100) trainingPriority = 0;
    if (weaponsPercentage<lowestPercentage) {
      lowestPercentage = weaponsPercentage;
      trainingPriority = 1;
    }
    if (flightPercentage<lowestPercentage) {
      lowestPercentage = flightPercentage;
      trainingPriority = 2;
    }
    if (navigationPercentage<lowestPercentage) {
      trainingPriority = 3;
    }

    // now see if any of the trainings are going to expire in the next 10 seconds
    if ( DockingTrainingEndTime && (CurrentTime+10000)>DockingTrainingEndTime ) {
      trainingPriority = 0;
    } else if ( WeaponsTrainingEndTime && (CurrentTime+10000)>WeaponsTrainingEndTime ) {
      trainingPriority = 1;
    } else if ( FlightTrainingEndTime && (CurrentTime+10000)>FlightTrainingEndTime ) {
      trainingPriority = 2;
    } else if ( NavigationTrainingEndTime && (CurrentTime+10000)>NavigationTrainingEndTime ) {
      trainingPriority = 3;
    }

    if (trainingPriority!=0xFF) {
      LastTimeTrainingCalloutMade = CurrentTime;
      byte randomOffset = CurrentTime%3;
      QueueNotification(SOUND_EFFECT_VP_DOCKING_TRAINING_REMINDER_1+trainingPriority+(randomOffset*4), 8);
      QueueNotification(GetNicknameSoundIndex(PlayerNickname[CurrentPlayer], false), 8);
    }
  } else {
    // nothing running
    //SOUND_EFFECT_VP_DOCKING_TRAINING_NUDGE_1
    byte trainingPriority = 0xFF;
    if ( (PlayerTrainingStatus[CurrentPlayer]&WEAPONS_TRAINING_ACHIEVED)==0 && CurrentTime>(LastTimeWeaponsTrainingHit+120000) ) {
      trainingPriority = 1;
    } else if ( (PlayerTrainingStatus[CurrentPlayer]&FLIGHT_TRAINING_ACHIEVED)==0 && CurrentTime>(LastTimeFlightTrainingHit+120000) ) {
      trainingPriority = 2;
    } else if ( (PlayerTrainingStatus[CurrentPlayer]&DOCKING_TRAINING_ACHIEVED)==0 && CurrentTime>(LastTimeDockingTrainingHit+120000) ) {
      trainingPriority = 0;
    } else if ( (PlayerTrainingStatus[CurrentPlayer]&NAVIGATION_TRAINING_ACHIEVED)==0 && CurrentTime>(LastTimeNavigationTrainingHit+120000) ) {
      trainingPriority = 3;
    }

    if (trainingPriority!=0xFF) {
      LastTimeTrainingCalloutMade = CurrentTime;
      byte randomOffset = CurrentTime%3;
      QueueNotification(SOUND_EFFECT_VP_DOCKING_TRAINING_NUDGE_1+trainingPriority+(randomOffset*4), 5);
      QueueNotification(GetNicknameSoundIndex(PlayerNickname[CurrentPlayer], false), 8);
    }
  }
*/  
}



byte CalculateBattleStageShots(byte stage) {
  byte numberOfShots = 1;
  if (stage==BATTLE_STAGE_SPINNER) {
    // Spinner shots
    numberOfShots = 50 + BattlesWon[CurrentPlayer] * 5;
  } else if (stage==BATTLE_STAGE_STANDUPS) {
    numberOfShots = 1 + (BattlesWon[CurrentPlayer]/2);
  } else if (stage==BATTLE_STAGE_BULLSEYE) {
    numberOfShots = 1 + (BattlesWon[CurrentPlayer]/5);
  } else if (stage==BATTLE_STAGE_SAUCER) {
    numberOfShots = 1;
  }

  return numberOfShots;
}


byte NumberOfSwitchHitsForBossStage(byte stageNum) {
  if (stageNum==BOSS_STAGE_FIRE_LANES) return 60;
  if (stageNum==BOSS_STAGE_POP_BUMPERS) return 50;
  if (stageNum==BOSS_STAGE_NUMBER_STANDUPS) return 40;
  if (stageNum==BOSS_STAGE_POWER_STANDUPS) return 30;
  return 75;
}


int CalculateShotsNeededForNextTraining(byte trainingType) {

  int returnShots;
  
  if (trainingType==WEAPONS_TRAINING) {
    returnShots = (int)NumberOfPopHitsForWeaponsTraining;   
    if (PlayerTrainingStatus[CurrentPlayer] & WEAPONS_TRAINING_ACHIEVED_MASK) {
      int additionalShots = (PlayerTrainingStatus[CurrentPlayer] & WEAPONS_TRAINING_ACHIEVED_MASK) / WEAPONS_TRAINING_ACHIEVED_SHIFT;
      returnShots += additionalShots * 15;
    }
  } else if (trainingType==FLIGHT_TRAINING) {
    returnShots = (int)NumberOfSpinnerHitsForFlightTraining;   
    if (PlayerTrainingStatus[CurrentPlayer] & FLIGHT_TRAINING_ACHIEVED_MASK) {
      int additionalShots = (PlayerTrainingStatus[CurrentPlayer] & FLIGHT_TRAINING_ACHIEVED_MASK) / FLIGHT_TRAINING_ACHIEVED_SHIFT;
      returnShots += additionalShots * 25;
    }
  }

  return returnShots;
}


void DetermineIfBossStageComplete() {
  byte stageIndex = BossBattleStage - 1;
  byte stageMask = 0x01 << stageIndex;

  if (BossHitsInStage[stageIndex] >= BossBattleShotThreshold[CurrentPlayer]) {
    BossBattleStagesDone |= stageMask;
  }
  
}


void ShowTrainingStatusInDisplays() {

  byte value1, value2, value3, value4;
  byte effect1, effect2, effect3, effect4;

  effect1 = DISPLAY_OVERRIDE_ANIMATION_CENTER;
  effect2 = DISPLAY_OVERRIDE_CENTER_FLASH_SLOW;
  effect3 = DISPLAY_OVERRIDE_ANIMATION_CENTER;
  effect4 = DISPLAY_OVERRIDE_CENTER_FLASH_SLOW;

  value1 = CalculateShotsNeededForNextTraining(FLIGHT_TRAINING) - SpinnerHits[CurrentPlayer];
  if (CurrentTime < (LastTimeFlightTrainingHit+3000)) effect1 = DISPLAY_OVERRIDE_CENTER_FLASH_SLOW;
  
  value2 = FlightTrainingHits[CurrentPlayer];
  // In the second box, we'll show flight training hits so far
  if (PlayerTrainingStatus[CurrentPlayer]&FLIGHT_TRAINING_RUNNING) {
    effect2 = DISPLAY_OVERRIDE_CENTER_FLASH_FAST;
  }
  
  value3 = CalculateShotsNeededForNextTraining(WEAPONS_TRAINING) - PopHits[CurrentPlayer];
  if (CurrentTime < (LastTimeWeaponsTrainingHit+3000)) effect3 = DISPLAY_OVERRIDE_CENTER_FLASH_SLOW;

  value4 = WeaponsTrainingHits[CurrentPlayer];
  // In the second box, we'll show flight training hits so far
  if (PlayerTrainingStatus[CurrentPlayer]&WEAPONS_TRAINING_RUNNING) {
    effect4 = DISPLAY_OVERRIDE_CENTER_FLASH_FAST;
  }

  Display_OverrideScoreDisplay(0, value1, effect1);
  Display_OverrideScoreDisplay(1, value2, effect2);
  Display_OverrideScoreDisplay(2, value3, effect3);
  Display_OverrideScoreDisplay(3, value4, effect4);
}


void LockInsteadOfBattle() {
  RightFlipperDown = 1;
  RightFlipperHeld = false;
  if (LockBall(OfferBattleSaucer)) {
    QueueNotification(SOUND_EFFECT_VP_BALL_LOCKED, 10);

/*
    if (ShipWeapons[CurrentPlayer]==0) {
      QueueNotification(SOUND_EFFECT_VP_BATTLE_3_REJECTED_WEAPONS, 10); 
    } else if (ShipThrusters[CurrentPlayer]==0) {
      QueueNotification(SOUND_EFFECT_VP_BATTLE_3_REJECTED_THRUSTERS, 10); 
    } else {
      if ((CurrentTime%4)==0) QueueNotification(SOUND_EFFECT_VP_BATTLE_3_REJECTED_UPGRADE, 10); 
      else QueueNotification(SOUND_EFFECT_VP_BATTLE_1_OR_2_REJECTED_1 + (CurrentTime%4 - 1), 10);
    }
*/    
    DisplaysNeedRefreshing = false;
    if (CountBallsInTrough()) {
      // we have a ball to serve
      if (RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)==0) RPU_PushToTimedSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, CurrentTime, true);
      LastTimeBallServed = CurrentTime;              
      BallSaveNumSeconds = BallSaveAfterLock;
      SetGameMode(GAME_MODE_WAIT_FOR_BALL);
    } else {
      // we don't have a ball to serve, so we need to release one
      // belonging to another player
      ReleaseOtherPlayersLock(OfferBattleSaucer);
      SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
    }
  } else {
    // the lock didn't work, so return the ball
    RPU_PushToSolenoidStack(SaucerSolenoids[OfferBattleSaucer], SaucerSolenoidStrength, true);
    SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
  }  
}


void AwardJackpot(byte increaseStep, boolean doubleJackpot = false) {
  if (!doubleJackpot) {
    Display_StartScoreAnimation( ((unsigned long)JackpotValue[CurrentPlayer] * ((unsigned long)PlayerRank[CurrentPlayer]+1) * 1000) * PlayfieldMultiplier, true);
    QueueNotification(SOUND_EFFECT_VP_JACKPOT_1 + CurrentTime%6, 5);
  } else {
    Display_StartScoreAnimation( ((unsigned long)JackpotValue[CurrentPlayer] * ((unsigned long)PlayerRank[CurrentPlayer]+1) * 2000) * PlayfieldMultiplier, true);
    QueueNotification(SOUND_EFFECT_VP_DOUBLE_JACKPOT, 5);
  }
  JackpotValue[CurrentPlayer] += increaseStep;
  LastTimeJackpotAdjusted = CurrentTime;
}


void ShowTimer() {
  if (ShowStatusInCreditsWindow) {
    byte timeToShow = (GameModeEndTime-CurrentTime)/1000;
    if ( (GameModeEndTime-CurrentTime)>99999 ) {
      timeToShow = 99;
    }
    boolean displayPhase = ((CurrentTime/250)%2)?true:false;
    RPU_SetDisplayCredits(timeToShow, displayPhase, false);
  }
}


void ShowBossHitsLeftInDisplays(byte hitsLeft) {
  boolean displayPhase = ((CurrentTime/200)%3)?true:false;
  RPU_SetDisplayCredits(hitsLeft, displayPhase, false);
}


void ShowBattlesRemaining() {
  if (ShowStatusInCreditsWindow) {
    byte brPhase = (CurrentTime/250)%2;
    byte numBattlesLeft = NumberOfBattlesBeforeWizard - BattlesWon[CurrentPlayer];
    RPU_SetDisplayCredits(numBattlesLeft, brPhase, false);
  }  
}


void KickBossSaucersWhenReady() {
  for (byte count=0; count<3; count++) {
    if (BossSaucerKickoutTime[count] && CurrentTime>BossSaucerKickoutTime[count]) {
      BossSaucerKickoutTime[count] = 0;
      RPU_PushToSolenoidStack(SaucerSolenoids[count], SaucerSolenoidStrength, true);
    }    
  }
}


// This function manages all timers, flags, and lights
int ManageGameMode() {
  int returnState = MACHINE_STATE_NORMAL_GAMEPLAY;

  boolean specialAnimationRunning = false;
  boolean statusRunning = false;
  boolean gameModeEndsWithDrain = GameMode!=GAME_MODE_WAIT_FOR_BALL;
  TimersPaused = false;

  CheckSaucersForBall();
  UpdateTimers();

  if (RPU_ReadSingleSwitchState(SW_RIGHT_FLIPPER)) {
    if (RightFlipperDown==0) {
      RightFlipperDown = CurrentTime;
    } else if (RightFlipperDown>1 && CurrentTime>(RightFlipperDown+1500)) {
      RightFlipperHeld = true;
    }
  } else {
    RightFlipperDown = 0;
    RightFlipperHeld = false;
  }

  switch ( GameMode ) {
    case GAME_MODE_SKILL_SHOT:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        LastTimePromptPlayed = CurrentTime;
        GameModeStage = 0;
        SetGeneralIlluminationOn(true);

        if (SkillShotChangesWhenHit==false) {
          SkillShotType[CurrentPlayer] = SKILL_SHOT_TYPE_ALWAYS_CHANGEABLE_LANE;
        }
        SkillShotChangedAfterLaunch = false;
        LastSkillShotChangedTime = CurrentTime;
        SkillShotLane = 0;
        if (SkillShotType[CurrentPlayer]==SKILL_SHOT_TYPE_FIXED_LANE) {
          SkillShotLane = SkillShotsHit[CurrentPlayer] % 4;
        }      
      }

      // The switch handler will award the skill shot
      // (when applicable) and this mode will move
      // to unstructured play when any valid switch is
      // recorded

      if (CurrentTime > (LastTimePromptPlayed + 20000)) {
        AlertPlayerUp();
        LastTimePromptPlayed = CurrentTime;
      }

      if (SkillShotType[CurrentPlayer]==SKILL_SHOT_TYPE_AUTO_CHANGING_LANE) {
        if (RPU_ReadSingleSwitchState(SW_RIGHT_FLIPPER)) {
          if (CurrentTime>(LastSkillShotChangedTime+1000)) {
            SkillShotLane = (SkillShotLane+1)%4;
            LastSkillShotChangedTime = CurrentTime;
          }
        } else {
          if (CurrentTime>(LastSkillShotChangedTime+250)) {
            SkillShotLane = (SkillShotLane+1)%4;
            LastSkillShotChangedTime = CurrentTime;
          }
        }
      }
      
      // If we've seen a tilt before plunge, then
      // we can show a countdown timer here
      if (LastTiltWarningTime) {
        if ( CurrentTime > (LastTiltWarningTime + 30000) ) {
          LastTiltWarningTime = 0;
        } else {
          byte secondsSinceWarning = (CurrentTime - LastTiltWarningTime) / 1000;
          for (byte count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
            if (count == CurrentPlayer && !statusRunning) Display_OverrideScoreDisplay(count%RPU_NUMBER_OF_PLAYER_DISPLAYS, 30 - secondsSinceWarning, DISPLAY_OVERRIDE_ANIMATION_CENTER);
          }
          DisplaysNeedRefreshing = true;
        }
      } else if (RightFlipperHeld) {
        byte displayPhase = (CurrentTime/2000)%2;
        if (displayPhase==0) {
          DisplaysNeedRefreshing = true;
          ShowTrainingStatusInDisplays();
        } else {
          DisplaysNeedRefreshing = false;
          Display_ClearOverride(0xFF);
        }          
      } else if (DisplaysNeedRefreshing) {
        DisplaysNeedRefreshing = false;
        if (!statusRunning) Display_ClearOverride(0xFF);
      } else {
        if (GameModeStage!=1) {
          if (!statusRunning) Display_ClearOverride(0xFF);
          GameModeStage = 1;
        }
      }

      if (BallFirstSwitchHitTime != 0) {
        Display_ClearOverride(0xFF);
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_WAIT_FOR_BALL:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
      }

      if (CurrentTime>(LastTimeBallServed+1500)) {
        LastTimeBallServed = CurrentTime;
        RPU_PushToSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, true);
      }

      if (RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) {
        BallFirstSwitchHitTime = 0;
        SetGameMode(GAME_MODE_SKILL_SHOT);
      }
      break;

    case GAME_MODE_UNSTRUCTURED_PLAY:
      // If this is the first time in this mode
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        DisplaysNeedRefreshing = true;
        if (DEBUG_MESSAGES) {
          Serial.write("Entering unstructured play\n");
        }
        SetGeneralIlluminationOn(true);
        GameModeStage = 0;
        
        // If we need to reset the game song
        if (ResetGameSong==1) {
          short curBackgroundSong = SOUND_EFFECT_BACKGROUND_SONG_1;
          if (CurrentBallInPlay>1 && CurrentBallInPlay<BallsPerGame) {
            curBackgroundSong = SOUND_EFFECT_BACKGROUND_SONG_1 + 1 + (CurrentTime%3);
          } else if (CurrentBallInPlay==BallsPerGame) {
            curBackgroundSong = SOUND_EFFECT_BACKGROUND_SONG_5;
          }
          PlayBackgroundSong(curBackgroundSong);
        } else if (ResetGameSong==2) {
          PlayBackgroundSong(SOUND_EFFECT_COOLDOWN_SONG_1 + CurrentTime%2);
        }
        ResetGameSong = 0;
        OfferBattleSaucer = 0xFF;
        SecondBattleSaucer = 0xFF;
        ThirdBattleSaucer = 0xFF;
        BattleStage = BATTLE_STAGE_OFF;
        BattleStageShots = 0;
        JackpotReady = false;

        // These time marks are used for voice notifications,
        // so we'll set them to the time this GameMode started
        // so we don't overload the player with messages
        LastTimePromptPlayed = CurrentTime;
        LastTimeBattleShotHit = CurrentTime;
        LastTimeLockShotHit = CurrentTime;
        LastTimeShipUpgraded = CurrentTime;
        LastTimeTrainingCalloutMade = CurrentTime;
        LastTimeWeaponsTrainingHit = CurrentTime;
        LastTimeFlightTrainingHit = CurrentTime;
        LastTrainingHitTime = CurrentTime;
      }

      if (CurrentTime > (GameModeStartTime+5000)) {
        if ( (PlayerTrainingStatus[CurrentPlayer] & TRAINING_RUNNING_MASK) || RightFlipperHeld) {
          byte displayPhase = (CurrentTime/2000)%2;
          if (displayPhase==0) {
            DisplaysNeedRefreshing = true;
            ShowTrainingStatusInDisplays();
          } else {
            DisplaysNeedRefreshing = false;
            Display_ClearOverride(0xFF);
          }
        } else if ( CurrentTime < (LastTrainingHitTime+4000) ) {
          DisplaysNeedRefreshing = true;
          ShowTrainingStatusInDisplays();
        } else if ( PlayfieldMultiplier>1 ) {
          for (byte count = 0; count < 4; count++) {
            if (count != CurrentPlayer) Display_OverrideScoreDisplay(count, PlayfieldMultiplier, DISPLAY_OVERRIDE_SYMMETRIC_BOUNCE);
          }
          DisplaysNeedRefreshing = true;
        } else if (DisplaysNeedRefreshing) {
          DisplaysNeedRefreshing = false;
          Display_ClearOverride(0xFF);
        }
      }

      ShowBattlesRemaining();
      
      //QueueTrainingCalloutIfNecessary();
      
      // Check to see if player has earned an extra ball lamp
      if (CurrentTime > (LastTimeAwardsChecked+1000) && ExtraBallAvailable[CurrentPlayer]==0) {
        LastTimeAwardsChecked = CurrentTime;
        // if extra ball criteria met
//        if () {
//          ExtraBallAvailable[CurrentPlayer] = 1;
//        }
      }

      break;


    case GAME_MODE_START_TRAINING:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;

#ifdef DEBUG_MESSAGES
        char buf[128];
        sprintf(buf, "Player start Training (1) = 0x%02X\n", PlayerTrainingStatus[CurrentPlayer]);
        Serial.write(buf);
#endif  
        
        if (PlayerTrainingStatus[CurrentPlayer] & WEAPONS_TRAINING_QUALIFIED) {
          WeaponsTrainingStartingHits = WeaponsTrainingHits[CurrentPlayer];
          PlayerTrainingStatus[CurrentPlayer] &= ~WEAPONS_TRAINING_QUALIFIED;
          PlayerTrainingStatus[CurrentPlayer] |= WEAPONS_TRAINING_RUNNING;
          WeaponsTrainingEndTime = CurrentTime + ((unsigned long)TrainingDuration * 1000);
          IncreasePlayfieldMultiplier(((unsigned long)TrainingDuration + 10) * 1000);
          //PopHits[CurrentPlayer] = 0;
          QueueNotification(SOUND_EFFECT_VP_STARTING_WEAPONS_TRAINING, 7);
        }

        if (PlayerTrainingStatus[CurrentPlayer] & FLIGHT_TRAINING_QUALIFIED) {
          FlightTrainingStartingHits = FlightTrainingHits[CurrentPlayer];
          PlayerTrainingStatus[CurrentPlayer] &= ~FLIGHT_TRAINING_QUALIFIED;
          PlayerTrainingStatus[CurrentPlayer] |= FLIGHT_TRAINING_RUNNING;
          FlightTrainingEndTime = CurrentTime + ((unsigned long)TrainingDuration * 1000);
          IncreasePlayfieldMultiplier(((unsigned long)TrainingDuration + 10) * 1000);
          //SpinnerHits[CurrentPlayer] = 0;
          QueueNotification(SOUND_EFFECT_VP_STARTING_FLIGHT_TRAINING, 7);
        }

#ifdef DEBUG_MESSAGES
        sprintf(buf, "Player start Training (2) = 0x%02X\n", PlayerTrainingStatus[CurrentPlayer]);
        Serial.write(buf);
#endif  

        GameModeEndTime = CurrentTime + 2000;
      }

      if (CurrentTime>GameModeEndTime) {
        if (OfferBattleSaucer!=0xFF) RPU_PushToSolenoidStack(SaucerSolenoids[OfferBattleSaucer], SaucerSolenoidStrength, true);
        OfferBattleSaucer = 0xFF;
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_OFFER_BATTLE_1:
    case GAME_MODE_OFFER_BATTLE_2:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 10000;
        DisplaysNeedRefreshing = true;
        GameModeStage = 0;
        LastTimePromptPlayed = 0;
        if (GameMode==GAME_MODE_OFFER_BATTLE_2) PlayBackgroundSong(SOUND_EFFECT_READY_FOR_BATTLE_2);
        else PlayBackgroundSong(SOUND_EFFECT_READY_FOR_BATTLE_1);
        Display_ClearOverride(0xFF);
      }
      TimersPaused = true;

      specialAnimationRunning = true;
      if (GameMode==GAME_MODE_OFFER_BATTLE_1) ShowLampAnimation(0, 20, CurrentTime, 18, false, false);
      else if (GameMode==GAME_MODE_OFFER_BATTLE_2) ShowLampAnimation(1, 20, CurrentTime, 18, false, false);
      else ShowLampAnimation(1, 10, CurrentTime, 18, false, false);

      if (GameModeStage==0 && CurrentTime>(GameModeStartTime+1000)) {
        GameModeStage = 1;
        if (GameMode==GAME_MODE_OFFER_BATTLE_1) QueueNotification(SOUND_EFFECT_VP_OFFER_BATTLE_1, 10);
        else if (GameMode==GAME_MODE_OFFER_BATTLE_2) QueueNotification(SOUND_EFFECT_VP_OFFER_BATTLE_2, 10);
        else QueueNotification(SOUND_EFFECT_VP_OFFER_BATTLE_3, 10);
      }

      for (byte count = 0; count < 4; count++) {
        if (count != CurrentPlayer) Display_OverrideScoreDisplay(count, (GameModeEndTime - CurrentTime) / 1000, DISPLAY_OVERRIDE_ANIMATION_CENTER);
      }

      if (RightFlipperHeld) {
        RightFlipperDown = 1;
        RightFlipperHeld = false;
        Display_ClearOverride(0xFF);
        if (GameMode==GAME_MODE_OFFER_BATTLE_1) SetGameMode(GAME_MODE_START_BATTLE_1);
        else if (GameMode==GAME_MODE_OFFER_BATTLE_2) SetGameMode(GAME_MODE_START_BATTLE_2);
        else SetGameMode(GAME_MODE_START_BATTLE_3);
      }

      if (GameModeEndTime && CurrentTime > GameModeEndTime && (RightFlipperDown==0)) {
        Display_ClearOverride(0xFF);
        // Player didn't take the battle, so we're
        // going to lock the ball and move on
        LockInsteadOfBattle();        
        ResetGameSong = 1;
      }
      break;
    case GAME_MODE_START_BATTLE_1:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 5000;
        GameModeStage = 0;
        LastTimePromptPlayed = 0;
        Audio.StopAllMusic();
        PlaySoundEffect(SOUND_EFFECT_POWER_COMPLETED);
        QueueNotification(SOUND_EFFECT_VP_BATTLE_1_INSTRUCTIONS, 10);
        BattleRefuelingTime = 0;
        JackpotValue[CurrentPlayer] += 20;
        BattlesPlayed[CurrentPlayer] |= 0x01;
      }
      TimersPaused = true;

      if (GameModeStage==0) {
        GameModeStage = 1;
        RPU_TurnOffAllLamps();
        SetGeneralIlluminationOn(false);
      } else {
        specialAnimationRunning = true;
        byte lampPhase = (CurrentTime-GameModeStartTime) / 200;
        FlashAnimationSteps(2, lampPhase, 50);

        byte numSeconds = ((CurrentTime-GameModeStartTime) / 1000);
        if (numSeconds > (GameModeStage-1)) {
          GameModeStage = numSeconds + 1;
          Audio.StopSound(SOUND_EFFECT_COUNTDOWN_1);
          PlaySoundEffect(SOUND_EFFECT_COUNTDOWN_1);
        }
      }
            
      if (GameModeEndTime && (CurrentTime > GameModeEndTime || RightFlipperDown>1)) {
        BattleStage = BATTLE_STAGE_SPINNER;
        BattleStageShots = CalculateBattleStageShots(BattleStage);
        BattleStandups = 0;
        BattleStandupsLastChange = 0;
        LastBattleStandupHit = 0xFF;
        LastBattleStandupHitTime = 0;
        // Announce the weapons and thrusters
        if (ShipWeapons[CurrentPlayer]) {
          QueueNotification(SOUND_EFFECT_VP_LEFT_SHIP + OfferBattleSaucer, 6);
          QueueNotification(SOUND_EFFECT_VP_HAS_WEAPONS_LEVEL_1 + (ShipWeapons[CurrentPlayer]-1), 6);
        }
        if (ShipThrusters[CurrentPlayer]) {
          QueueNotification(SOUND_EFFECT_VP_LEFT_SHIP + OfferBattleSaucer, 6);
          QueueNotification(SOUND_EFFECT_VP_HAS_THRUSTERS_LEVEL_1 + (ShipThrusters[CurrentPlayer]-1), 6);
        }
        
        PlayBackgroundSong(SOUND_EFFECT_BATTLE_SONG_3);
        PlayerLocks[CurrentPlayer] &= ~(0x01 << OfferBattleSaucer);
        RPU_PushToSolenoidStack(SaucerSolenoids[OfferBattleSaucer], SaucerSolenoidStrength, true);
        LastTimeBattleShotHit = CurrentTime;
        SetGameMode(GAME_MODE_BATTLE_1);
      }
      break;
    case GAME_MODE_BATTLE_1:
      if (GameModeStartTime == 0) {        
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + ((unsigned long)BattleFuelTime * 1000);
        GameModeStage = 0;
        LastTimePromptPlayed = 0;
        RefuelingMessagePlayed = false;
        JackpotReady = false;
        
        // Stage 1 - spinner to reach
        // Stage 2 - standups to target
        // Stage 3 - center standup to hit
        // Stage 4 - saucer for landing
      }
      TimersPaused = true;

      ShowTimer();

      if (BattleRefuelingTime) {
        if (LastTimePromptPlayed==0) {
          LastTimePromptPlayed = CurrentTime;
        }

        if (CurrentTime>(BattleRefuelingTime+2000)) {
          // refueling done
          BattleRefuelingTime = 0;
          LastTimePromptPlayed = 0;
          RPU_PushToSolenoidStack(SaucerSolenoids[OfferBattleSaucer], SaucerSolenoidStrength, true);
        } else {
          unsigned long timeIncrement = ((unsigned long) 2000) / (BattleFuelTime/2);
          if (CurrentTime > (LastTimePromptPlayed+timeIncrement)) {
            LastTimePromptPlayed = CurrentTime;
            if (GameModeEndTime < (CurrentTime + ((unsigned long)BattleFuelTime * 1500)) ) {
              GameModeEndTime += 1000;              
              PlaySoundEffect(SOUND_EFFECT_REFUELING);
            } else {
              // max fuel capacity
              BattleRefuelingTime = 0;
              LastTimePromptPlayed = 0;
              RPU_PushToSolenoidStack(SaucerSolenoids[OfferBattleSaucer], SaucerSolenoidStrength, true);
            }
          }
        }
      }

      // Check to see if we've finished a stage
      if (BattleStageShots==0) {
        BattleStage += 1;
        BattleStageShots = CalculateBattleStageShots(BattleStage);
        if (BattleStage==BATTLE_STAGE_STANDUPS) {
          AwardJackpot(10);
          BattleStandupsLastChange = CurrentTime;
          for (byte count=0; count<(ShipWeapons[CurrentPlayer]+1); count++) {
            BattleStandups |= 1;
            BattleStandups *= 2;
          }
          QueueNotification(SOUND_EFFECT_VP_BATTLE_STANDUPS, 7);
        } else if (BattleStage==BATTLE_STAGE_BULLSEYE) {
          AwardJackpot(15);
          QueueNotification(SOUND_EFFECT_VP_BATTLE_BULLSEYE, 7);          
        } else if (BattleStage==BATTLE_STAGE_SAUCER) {
          AwardJackpot(15);
          QueueNotification(SOUND_EFFECT_VP_BATTLE_SAUCER, 7);
        } else if (BattleStage==BATTLE_STAGE_WON) {
          AwardJackpot(0, true);
          RPU_SetDisplayCredits(Credits, !FreePlayMode);
          Display_ClearOverride();
          SetGameMode(GAME_MODE_BATTLE_1_WON);
        }

      }

      if (LastBattleStandupHitTime && CurrentTime>(LastBattleStandupHitTime+3000)) {
        LastBattleStandupHit = 0xFF;
        LastBattleStandupHitTime = 0;
      }

      if (BattleStage==BATTLE_STAGE_STANDUPS && CurrentTime>(BattleStandupsLastChange+300)) {
        BattleStandupsLastChange = CurrentTime;
        byte shiftDirection = (CurrentTime/3000)%2;
        if (shiftDirection) {
          byte carryBit = (BattleStandups & 0x20) ? 0x01 : 0x00;
          BattleStandups = ((BattleStandups * 2) & 0x3F) | carryBit;
        } else {
          byte carryBit = (BattleStandups & 0x01) ? 0x20 : 0x00;
          BattleStandups = (BattleStandups / 2) | carryBit;
        }
        //BattleStandups *= 2; 
      }

      if (CurrentTime < (LastTimeBattleShotHit+3000)) {
        Display_OverrideScoreDisplay(CurrentPlayer, BattleStageShots, DISPLAY_OVERRIDE_ANIMATION_CENTER);
        DisplaysNeedRefreshing = true;
      } else if (DisplaysNeedRefreshing) {
        DisplaysNeedRefreshing = false;
        Display_ClearOverride(0xFF);
      }

      if (CurrentTime>(GameModeStartTime + 5000) && GameModeStage==0) {
        GameModeStage = 1;
        if (BattleStage==BATTLE_STAGE_SPINNER) QueueNotification(SOUND_EFFECT_VP_BATTLE_SPINNER, 7);
        // Could have other prompts here (in case we dropped into Battle 1 and 
        // have a different stage going
      }

      if ( (CurrentTime + 10000)>GameModeEndTime) {
        if (GameModeStage==1) {
          GameModeStage = 2;
          QueueNotification(SOUND_EFFECT_VP_BATTLE_1_TEN_SECONDS, 7);
        }
        for (byte count = 0; count < 4; count++) {
          if (count != CurrentPlayer) Display_OverrideScoreDisplay(count, (GameModeEndTime - CurrentTime) / 1000, DISPLAY_OVERRIDE_ANIMATION_CENTER);
        }
      } else {
        // If they refueled, allow the ten seconds message again
        if (GameModeStage==2) GameModeStage = 1;
      }

      if (GameModeEndTime && (CurrentTime > GameModeEndTime)) {
        RPU_SetDisplayCredits(Credits, !FreePlayMode);
        Display_ClearOverride(0xFF);
        SetGameMode(GAME_MODE_BATTLE_1_LOST);
      }
      break;

    case GAME_MODE_BATTLE_1_WON:
      if (GameModeStartTime==0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 1000;
        if (ResetShipWeaponsEachBattle) {
          ShipWeapons[CurrentPlayer] = 0;
          ShipThrusters[CurrentPlayer] = 0;
        }
        QueueNotification(SOUND_EFFECT_VP_BATTLE_1_COMPLETED, 8);
        BattleStage = BATTLE_STAGE_OFF;
        BattleStageShots = 0xFF;
        BattleStandups = 0;
        BattleStandupsLastChange = 0;
        LastBattleStandupHit = 0xFF;
        LastBattleStandupHitTime = 0;
        BattlesWon[CurrentPlayer] += 1;
        // We should announce how many battles have been completed

        byte numBattlesLeft = NumberOfBattlesBeforeWizard - BattlesWon[CurrentPlayer];
        if (numBattlesLeft==0) {
          SetGameMode(GAME_MODE_BOSS_BATTLE_START);          
        } else {
          QueueNotification(SOUND_EFFECT_VP_ENEMIES_LEFT_1 + (numBattlesLeft-1), 10);
        }

      }
      TimersPaused = true;

      if (GameModeEndTime && (CurrentTime>GameModeEndTime)) {
        if (BattleOneFinishBehavior==BATTLE_ONE_FINISH_OFFER_NEXT) {
          PlayerLocks[CurrentPlayer] |= (0x01 << OfferBattleSaucer);
          SetGameMode(GAME_MODE_OFFER_BATTLE_1);
        } else {
          if (BattleOneFinishBehavior==BATTLE_ONE_FINISH_REQUALIFY) {
            PlayerLocks[CurrentPlayer] |= (0x01 << OfferBattleSaucer);
          }
          // if we're set to BATTLE_ONE_FINISH_EJECT, then they don't get the free lock          
          RPU_PushToSolenoidStack(SaucerSolenoids[OfferBattleSaucer], SaucerSolenoidStrength, true);
          ResetGameSong = 1;
          SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
        }
      }    
      break;
    case GAME_MODE_BATTLE_1_LOST:
      if (GameModeStartTime==0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 1000;
        if (ResetShipWeaponsEachBattle) {
          ShipWeapons[CurrentPlayer] = 0;
          ShipThrusters[CurrentPlayer] = 0;
        }
        QueueNotification(SOUND_EFFECT_VP_BATTLE_1_FAILED, 8);
      }
      TimersPaused = true;

      if (GameModeEndTime && (CurrentTime>GameModeEndTime)) {
        ResetGameSong = 1;
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }    
      break;
    case GAME_MODE_START_BATTLE_2:
    case GAME_MODE_START_BATTLE_3:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 4000;
        GameModeStage = 0;
        LastTimePromptPlayed = 0;
        Audio.StopAllMusic();
        PlaySoundEffect(SOUND_EFFECT_POWER_COMPLETED);
        BattleRefuelingTime = 0;
        JackpotValue[CurrentPlayer] += 20;
        if (GameMode==GAME_MODE_START_BATTLE_2) {
          IncreasePlayfieldMultiplier(5000);
          BattlesPlayed[CurrentPlayer] |= 0x02;
        } else if (GameMode==GAME_MODE_START_BATTLE_3) {
          IncreasePlayfieldMultiplier(5000);
          BattlesPlayed[CurrentPlayer] |= 0x04;
        } 
      }
      TimersPaused = true;

      if (GameModeStage==0) {
        GameModeStage = 1;
        RPU_TurnOffAllLamps();
        SetGeneralIlluminationOn(false);
      } else {
        specialAnimationRunning = true;
        byte lampPhase = (CurrentTime-GameModeStartTime) / 200;
        FlashAnimationSteps(2, lampPhase, 50);
        byte numSeconds = ((CurrentTime-GameModeStartTime) / 1000);
        if (numSeconds > (GameModeStage-1)) {
          GameModeStage = numSeconds + 1;
          Audio.StopSound(SOUND_EFFECT_COUNTDOWN_1);
          PlaySoundEffect(SOUND_EFFECT_COUNTDOWN_1);
        }
      }
            
      if (GameModeEndTime && (CurrentTime > GameModeEndTime || RightFlipperDown>1)) {
        BattleStage = BATTLE_STAGE_SPINNER;
        BattleStageShots = CalculateBattleStageShots(BattleStage);
        BattleStandups = 0;
        BattleStandupsLastChange = 0;
        LastBattleStandupHit = 0xFF;
        LastBattleStandupHitTime = 0;
        if (GameMode==GAME_MODE_START_BATTLE_2) {
          SetGameMode(GAME_MODE_BATTLE_2);
        } else if (GameMode==GAME_MODE_START_BATTLE_3) {
          SetGameMode(GAME_MODE_BATTLE_3);
        }
      }
      break;
    case GAME_MODE_BATTLE_2:
    case GAME_MODE_BATTLE_3:
      if (GameModeStartTime == 0) {        
        GameModeStartTime = CurrentTime;
        //GameModeEndTime = 0;
        GameModeStage = 0;
        LastTimePromptPlayed = 0;
        SetBallSave(0);
        
        PlayBackgroundSong(SOUND_EFFECT_BATTLE_SONG_4);
        PlayerLocks[CurrentPlayer] &= ~(0x01 << OfferBattleSaucer);
        RPU_PushToSolenoidStack(SaucerSolenoids[OfferBattleSaucer], SaucerSolenoidStrength, true);
        NumberOfBallsInPlay = 1;

        // Find the other locked ball and eject it too
        for (byte count=0; count<3; count++) {
          if (PlayerLocks[CurrentPlayer] & (BALL_LEFT_LOCK_ENGAGED<<count)) {
            PlayerLocks[CurrentPlayer] &= ~(BALL_LEFT_LOCK_ENGAGED << count);
            MachineLocks &= ~(BALL_LEFT_LOCK_ENGAGED << count);
            RPU_PushToSolenoidStack(SaucerSolenoids[count], SaucerSolenoidStrength, true);
            NumberOfBallsInPlay += 1;
          }
        }
        if (MachineLocks) {
          NumberOfBallsLocked = 1;
        } else {
          NumberOfBallsLocked = 0;
        }
        LastTimeBattleShotHit = CurrentTime;

        QueueNotification(SOUND_EFFECT_VP_BATTLE_2_SPINNER, 6);
      }
      TimersPaused = true;

      ShowBattlesRemaining();

      // Check to see if we've finished a stage
      if (BattleStageShots==0) {
        BattleStage += 1;
        BattleStageShots = CalculateBattleStageShots(BattleStage);
        if (BattleStage==BATTLE_STAGE_STANDUPS) {
          AwardJackpot(10);
          BattleStandupsLastChange = CurrentTime;
          for (byte count=0; count<(ShipWeapons[CurrentPlayer]+1); count++) {
            BattleStandups |= 1;
            BattleStandups *= 2;
          }
          QueueNotification(SOUND_EFFECT_VP_BATTLE_STANDUPS, 7);
        } else if (BattleStage==BATTLE_STAGE_BULLSEYE) {
          AwardJackpot(15);
          QueueNotification(SOUND_EFFECT_VP_BATTLE_BULLSEYE, 7);          
        } else if (BattleStage==BATTLE_STAGE_SAUCER) {
          AwardJackpot(15);
          BattlesWon[CurrentPlayer] += 1;
          byte numBattlesLeft = NumberOfBattlesBeforeWizard - BattlesWon[CurrentPlayer];
          if (numBattlesLeft==0) {
            SetGameMode(GAME_MODE_BOSS_BATTLE_START);
          } else {          
            QueueNotification(SOUND_EFFECT_VP_ENEMIES_LEFT_1 + (numBattlesLeft-1), 10);
            BattleStage = BATTLE_STAGE_SPINNER;
            BattleStageShots = CalculateBattleStageShots(BattleStage);
            BattleStandups = 0;
            BattleStandupsLastChange = 0;
            LastBattleStandupHit = 0xFF;
            LastBattleStandupHitTime = 0;
            JackpotReady = true;
          }
        }
      }

      if (LastBattleStandupHitTime && CurrentTime>(LastBattleStandupHitTime+3000)) {
        LastBattleStandupHit = 0xFF;
        LastBattleStandupHitTime = 0;
      }

      if (BattleStage==BATTLE_STAGE_STANDUPS && CurrentTime>(BattleStandupsLastChange+50*NumberOfBallsInPlay)) {
        BattleStandupsLastChange = CurrentTime;
        byte shiftDirection = (CurrentTime/2000)%2;
        if (GameMode==GAME_MODE_BATTLE_3) shiftDirection = (CurrentTime/1000)%2;
        if (shiftDirection) {
          byte carryBit = (BattleStandups & 0x20) ? 0x01 : 0x00;
          BattleStandups = ((BattleStandups * 2) & 0x3F) | carryBit;
        } else {
          byte carryBit = (BattleStandups & 0x01) ? 0x20 : 0x00;
          BattleStandups = (BattleStandups / 2) | carryBit;
        }
        //BattleStandups *= 2; 
      }

      if (CurrentTime < (LastTimeBattleShotHit+3000)) {
        Display_OverrideScoreDisplay(CurrentPlayer, BattleStageShots, DISPLAY_OVERRIDE_ANIMATION_CENTER);
        DisplaysNeedRefreshing = true;
      } else if (DisplaysNeedRefreshing) {
        DisplaysNeedRefreshing = false;
        Display_ClearOverride(0xFF);
      }

      if (CurrentTime>(GameModeStartTime + 5000) && GameModeStage==0) {
        GameModeStage = 1;
      }

      if (GameModeEndTime==0 && NumberOfBallsInPlay==1) {
        Display_ClearOverride(0xFF);
        JackpotReady = false;
        SetGameMode(GAME_MODE_BATTLE_1); // Drop down to battle 1 if the player drains
      }
      break;

    case GAME_MODE_BOSS_BATTLE_START:
      if (GameModeStartTime == 0) {        
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 4000;
        GameModeStage = 0;
        LastTimePromptPlayed = 0;
        PlayBackgroundSong(SOUND_EFFECT_BATTLE_SONG_5);
        EjectAllBallsFromSaucers();
        RPU_SetDisableFlippers(true);
        if (DEBUG_MESSAGES) Serial.write("Boss battle start\n");
      }
      gameModeEndsWithDrain = false;
      
      if (GameModeStage==0) {
        if (CurrentTime>=(GameModeStartTime+1200)) {
          GameModeStage = 1;
        } else {
          specialAnimationRunning = true;        
          byte lampPhase = (CurrentTime-GameModeStartTime) / 50;
          FlashAnimationSteps(1, lampPhase, 50);
        }
      } else if (GameModeStage==1) {
        RPU_TurnOffAllLamps();
        ShowHeadAndApronLamps();
        GameModeStage += 1;        
      }

      if (CurrentTime>GameModeEndTime) {
        SetGameMode(GAME_MODE_BOSS_BATTLE_WAIT_FOR_BALLS);   
      }
      break;

    case GAME_MODE_BOSS_BATTLE_WAIT_FOR_BALLS:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
      }
      if (CountBallsInTrough()==TotalBallsLoaded) {
        SetGameMode(GAME_MODE_BOSS_BATTLE);   
      }
      break;

    case GAME_MODE_BOSS_BATTLE:
      if (GameModeStartTime == 0) {        
        if (DEBUG_MESSAGES) Serial.write("Boss battle\n");
        RPU_EnableSolenoidStack();
        RPU_SetDisableFlippers(false);
        GameModeStartTime = CurrentTime;
        //GameModeEndTime = CurrentTime + 3000;
        GameModeStage = 0;
        LastTimePromptPlayed = 0;
        BossBattleStage = 1;
        BossBattleTimer = 0;
        BossBattleFuelBurn = false;
        BossBattleBallInShooter = 0;
        BossBattleStagesDone = 0;
        BossBattleBonus = 10000;
        SwitchHitsInMode = 0;
        for (byte count=0; count<BOSS_STAGE_TOTAL_STAGES; count++) {
          BossHitsInStage[count] = 0;
        }
        for (byte count=0; count<3; count++) BossSaucerKickoutTime[count] = 0;
      }
      gameModeEndsWithDrain = false;

      // Perpetually keep all balls in play
      if ( CountBallsInTrough() && (CurrentTime > (LastTimeBallServed+3000)) && !RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) {
        // only serve a ball every 3 seconds
        RPU_PushToTimedSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, CurrentTime, true);
        LastTimeBallServed = CurrentTime;
        // Every time we kick a ball, the player is penalized 
        // in their total time left for the battle
        if (BossBattleTimer>(BOSS_BATTLE_LOST_SHIP_PENALTY*2)) {
          BossBattleTimer -= BOSS_BATTLE_LOST_SHIP_PENALTY;
        }
      }

      if (GameModeStage==0 && SwitchHitsInMode!=0) {
        BossBattleTimer = BOSS_BATTLE_TOTAL_TIME;
        GameModeStage = 1;
        if (DEBUG_MESSAGES) {
          Serial.write("Saw a switch, starting timer\n");        
        }
      } else if (GameModeStage==1) {
        // Until the player has crossed the threshold of, show the bonus score flashing
        byte alternateDisplayEffect = DISPLAY_OVERRIDE_ANIMATION_CENTER;
        if (BossBattleFuelBurn) alternateDisplayEffect = DISPLAY_OVERRIDE_CENTER_FLASH_FAST;
        for (byte count=0; count<4; count++) {
          if (count==CurrentPlayer) Display_OverrideScoreDisplay(count, BossBattleBonus, DISPLAY_OVERRIDE_ANIMATION_FLUTTER);
          else Display_OverrideScoreDisplay(count, BossBattleTimer/1000, alternateDisplayEffect);
        }
        if (BossBattleStagesDone==BOSS_BATTLE_ALL_STAGES) GameModeStage = 2;
      } else if (GameModeStage==2) {
        // The player has crossed the threshold of shots needed so the bonus
        // score will be solid
        byte alternateDisplayEffect = DISPLAY_OVERRIDE_ANIMATION_CENTER;
        if (BossBattleFuelBurn) alternateDisplayEffect = DISPLAY_OVERRIDE_CENTER_FLASH_FAST;
        for (byte count=0; count<4; count++) {
          if (count==CurrentPlayer) Display_OverrideScoreDisplay(count, BossBattleBonus, DISPLAY_OVERRIDE_ANIMATION_FLUTTER);
          else Display_OverrideScoreDisplay(count, BossBattleTimer/1000, alternateDisplayEffect);
        }
      }

      if (SwitchHitsInMode>=NumberOfSwitchHitsForBossStage(BossBattleStage)) {
        SwitchHitsInMode = 0;
        BossBattleStage += 1;
        if (BossBattleStage > BOSS_STAGE_TOTAL_STAGES) {
          BossBattleStage = BOSS_STAGE_FIRE_LANES;
        }
      } else {
        unsigned long numSwitchesLeft = NumberOfSwitchHitsForBossStage(BossBattleStage) - SwitchHitsInMode;
        if (numSwitchesLeft<100) {
          ShowBossHitsLeftInDisplays(numSwitchesLeft);
        }
      }

      KickBossSaucersWhenReady();

      if (GameModeStage!=0 && BossBattleTimer==0) {
        EjectAllBallsFromSaucers();
        RPU_SetDisableFlippers(true);
        RPU_TurnOffAllLamps();
        ShowHeadAndApronLamps();
        SetGameMode(GAME_MODE_BOSS_BATTLE_ENDING);
      }
//      if (CurrentTime>GameModeEndTime) {
//        SetGameMode(GAME_MODE_BOSS_BATTLE_LOST);
//      }
      break;
    case GAME_MODE_BOSS_BATTLE_ENDING:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
      }
      if (CountBallsInTrough()) {
        byte totalBallsAccountedFor = CountBallsInTrough() + RPU_ReadSingleSwitchState(SW_SHOOTER_LANE) ? 1 : 0;
        if (totalBallsAccountedFor==TotalBallsLoaded) {
          // This mode has finished, so we can move on to 
          // GAME_MODE_BOSS_BATTLE_LOST or GAME_MODE_BOSS_BATTLE_WON
          if (BossBattleStagesDone==BOSS_BATTLE_ALL_STAGES) SetGameMode(GAME_MODE_BOSS_BATTLE_WON);
          else SetGameMode(GAME_MODE_BOSS_BATTLE_LOST);
          
        }
      }
      break;
    case GAME_MODE_BOSS_BATTLE_WON:
      if (GameModeStartTime == 0) {        
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 1000;
        GameModeStage = 0;
        LastTimePromptPlayed = 0;
        if (!RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) {
          RPU_PushToTimedSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, CurrentTime, true);
          LastTimeBallServed = CurrentTime;
          NumberOfBallsInPlay = 1;
          NumberOfBallsLocked = 0;
          PlayerLocks[CurrentPlayer] = 0;
          MachineLocks = 0;
        }
        Display_StartScoreAnimation( BossBattleBonus * PlayfieldMultiplier, true, DISPLAY_JACKPOT_ANIMATION_MAJOR_TICKS);
      }
      
      if (GameModeStage==0) {
        if (CurrentTime>=(GameModeStartTime+1200)) {
          GameModeStage = 1;
        } else {
          specialAnimationRunning = true;        
          byte lampPhase = (CurrentTime-GameModeStartTime) / 50;
          FlashAnimationSteps(1, lampPhase, 50);
        }
      }

      if (CurrentTime>GameModeEndTime) {
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);   
      }
      break;    
    case GAME_MODE_BOSS_BATTLE_LOST:
      if (GameModeStartTime == 0) {        
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 1000;
        GameModeStage = 0;
        LastTimePromptPlayed = 0;
        if (!RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) {
          RPU_PushToTimedSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, CurrentTime, true);
          LastTimeBallServed = CurrentTime;
          NumberOfBallsInPlay = 1;
          NumberOfBallsLocked = 0;
          PlayerLocks[CurrentPlayer] = 0;
          MachineLocks = 0;
        }
        Display_StartScoreAnimation( (BossBattleBonus/5) * PlayfieldMultiplier, true, DISPLAY_JACKPOT_ANIMATION_MAJOR_TICKS);
      }
      
      if (GameModeStage==0) {
        if (CurrentTime>=(GameModeStartTime+1200)) {
          GameModeStage = 1;
        } else {
          specialAnimationRunning = true;        
          byte lampPhase = (CurrentTime-GameModeStartTime) / 50;
          FlashAnimationSteps(1, lampPhase, 50);
        }
      }

      if (CurrentTime>GameModeEndTime) {
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);   
      }
      break;    
  }

  if ( !statusRunning && !specialAnimationRunning && NumTiltWarnings <= MaxTiltWarnings ) {
    ShowBonusLamps();
    ShowPopBumperLamps();
    ShowShootAgainLamp();
    ShowFireLamps();
    ShowLockLamps();
    ShowPowerLamps();
    ShowTargetLamps();
    ShowLaneLamps();
    ShowPlayerLamps();
  }

  if (Display_UpdateDisplays(0xFF, false, (BallFirstSwitchHitTime == 0) ? true : false, (BallFirstSwitchHitTime > 0 && ((CurrentTime - Display_GetLastTimeScoreChanged()) > 2000)) ? true : false)) {
    Audio.StopSound(SOUND_EFFECT_SCORE_TICK);
    PlaySoundEffect(SOUND_EFFECT_SCORE_TICK);
  }

  MoveBallFromOutholeToRamp();

  // Check to see if ball is in the outhole
  byte ballsExpectedInTrough = (TotalBallsLoaded - (NumberOfBallsInPlay + NumberOfBallsLocked));
  
  if ( gameModeEndsWithDrain && CountBallsInTrough()>ballsExpectedInTrough ) {

    if (BallTimeInTrough == 0) {
      // If this is the first time we're seeing too many balls in the trough, we'll wait to make sure
      // everything is settled
      BallTimeInTrough = CurrentTime;
    } else {

      // Make sure the ball stays on the sensor for at least
      // 0.5 seconds to be sure that it's not bouncing or passing through
      if ((CurrentTime - BallTimeInTrough) > 750) {

        if ((BallFirstSwitchHitTime == 0 && NumTiltWarnings <= MaxTiltWarnings)) {
          // Nothing hit yet, so return the ball to the player
          RPU_PushToTimedSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, CurrentTime);
          BallTimeInTrough = 0;
          returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
        } else {
          // if we haven't used the ball save, and we're under the time limit, then save the ball
          if (BallSaveEndTime && CurrentTime < (BallSaveEndTime + BALL_SAVE_GRACE_PERIOD)) {
            RPU_PushToTimedSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, CurrentTime + 100);

            RPU_SetLampState(LAMP_SHOOT_AGAIN, 0);
            RPU_SetLampState(LAMP_HEAD_SHOOT_AGAIN, 0);
            BallTimeInTrough = CurrentTime;
            returnState = MACHINE_STATE_NORMAL_GAMEPLAY;

            if (NumberOfBallSavesRemaining && NumberOfBallSavesRemaining != 0xFF) {
              NumberOfBallSavesRemaining -= 1;
              if (NumberOfBallSavesRemaining == 0) {
                BallSaveEndTime = 0;
                if (DEBUG_MESSAGES) {
                  Serial.write("Last ball save\n");
                }
                QueueNotification(SOUND_EFFECT_VP_BALL_SAVE, 10);
              } else {
                if (DEBUG_MESSAGES) {
                  Serial.write("Not last ball save\n");
                }                
              }
            }

          } else {

            NumberOfBallsInPlay -= 1;
            if (NumberOfBallsInPlay == 0) {
              Display_ClearOverride(0xFF);
              Audio.StopAllAudio();
              //PlaySoundEffect(SOUND_EFFECT_BALL_OVER);              
              returnState = MACHINE_STATE_COUNTDOWN_BONUS;
            }
          }
        }
      }
    }
  } else {
    BallTimeInTrough = 0;
  }


  if (TimersPaused) {
    // increaes end time of anything that should be extended
    unsigned long timeAdded = (CurrentTime - LastTimeThroughLoop);
    if (PlayfieldMultiplierExpirationTime) PlayfieldMultiplierExpirationTime += timeAdded;
    if (WeaponsTrainingEndTime) WeaponsTrainingEndTime += timeAdded;
    if (FlightTrainingEndTime) FlightTrainingEndTime += timeAdded;
  }
  if (BossBattleTimer) {
    unsigned long timeToRemove = (CurrentTime - LastTimeThroughLoop);
    if (BossBattleFuelBurn) timeToRemove *= 2;

    if (timeToRemove>BossBattleTimer) BossBattleTimer = 0;
    else BossBattleTimer -= timeToRemove;
  }

  LastLoopTick = CurrentTime;
  LastTimeThroughLoop = CurrentTime;
  return returnState;
}



unsigned long CountdownStartTime = 0;
unsigned long LastCountdownReportTime = 0;
unsigned long BonusCountDownEndTime = 0;
byte DecrementingBonusCounter;
byte IncrementingBonusXCounter;
byte TotalBonus = 0;
byte TotalBonusX = 0;
byte BonusTimingIndex;
byte BonusSoundIndex;
boolean CountdownBonusHurryUp = false;
int LastBonusSoundPlayed = 0;

int CountDownDelayTimes[] = {200, 185, 170, 155, 140, 115, 100, 85, 70, 50};

int CountdownBonus(boolean curStateChanged) {

  // If this is the first time through the countdown loop
  if (curStateChanged) {

    // Turn off solenoids
//    RPU_DisableSolenoidStack();
// (can't disable pops/slings on Williams without disabling flippers

    CountdownStartTime = CurrentTime;
    LastCountdownReportTime = CurrentTime;
    ShowBonusLamps();
    IncrementingBonusXCounter = 1;
    DecrementingBonusCounter = Bonus[CurrentPlayer];
    TotalBonus = Bonus[CurrentPlayer];
    TotalBonusX = BonusX[CurrentPlayer];
    CountdownBonusHurryUp = false;
    BonusSoundIndex = 0;
    BonusTimingIndex = 0;
//    LastBonusSoundPlayed = 0;

    BonusCountDownEndTime = 0xFFFFFFFF;
    // Some sound cards have a special index
    // for a "sound" that will turn off
    // the current background drone or currently
    // playing sound
    Audio.PlaySoundCardWhenPossible(19 * 256, CurrentTime, 0, 500, 10);
  }

  if (RPU_ReadSingleSwitchState(SW_RIGHT_FLIPPER)) CountdownBonusHurryUp = true;

  unsigned long countdownDelayTime = (unsigned long)(CountDownDelayTimes[BonusTimingIndex/2]);
  if (CountdownBonusHurryUp && countdownDelayTime > ((unsigned long)CountDownDelayTimes[9])) countdownDelayTime = CountDownDelayTimes[9];

  if ((CurrentTime - LastCountdownReportTime) > countdownDelayTime) {

    if (DecrementingBonusCounter) {

      // Only give sound & score if this isn't a tilt
      if (NumTiltWarnings <= MaxTiltWarnings) {
        int soundToPlay = SOUND_EFFECT_BONUS_COUNT_0 + (int)(BonusSoundIndex);
        if (soundToPlay>SOUND_EFFECT_BONUS_COUNT_9) soundToPlay = SOUND_EFFECT_BONUS_COUNT_9;
        if (LastBonusSoundPlayed!=0) Audio.StopSound(LastBonusSoundPlayed);
        PlaySoundEffect(soundToPlay);
        LastBonusSoundPlayed = soundToPlay;
        CurrentScores[CurrentPlayer] += 1000;
        if (DEBUG_MESSAGES) {
          char buf[128];
          sprintf(buf, "Bonus Sound %d\n", soundToPlay);
          Serial.write(buf);
        }
      }

      BonusSoundIndex = (BonusSoundIndex + 1)%10;
      BonusTimingIndex += 1;
      if (BonusTimingIndex>19) BonusTimingIndex = 19;
      DecrementingBonusCounter -= 1;
      Bonus[CurrentPlayer] = DecrementingBonusCounter;
      ShowBonusLamps();

    } else if (BonusCountDownEndTime == 0xFFFFFFFF) {
      IncrementingBonusXCounter += 1;
      if (BonusX[CurrentPlayer] > 1) {
        BonusSoundIndex = 0;
        DecrementingBonusCounter = TotalBonus;
        Bonus[CurrentPlayer] = TotalBonus;
        ShowBonusLamps();
        BonusX[CurrentPlayer] -= 1;
        if (BonusX[CurrentPlayer] == 9) BonusX[CurrentPlayer] = 8;
      } else {
        BonusX[CurrentPlayer] = TotalBonusX;
        Bonus[CurrentPlayer] = TotalBonus;
        BonusCountDownEndTime = CurrentTime + 1000;
      }
    }
    LastCountdownReportTime = CurrentTime;
  }

  if (CurrentTime > BonusCountDownEndTime) {

    if (DEBUG_MESSAGES) {
      Serial.write("Count down over, moving to ball over\n");
    }
    // Reset any lights & variables of goals that weren't completed
    BonusCountDownEndTime = 0xFFFFFFFF;
    return MACHINE_STATE_BALL_OVER;
  }

  return MACHINE_STATE_COUNTDOWN_BONUS;
}



void CheckHighScores() {
  unsigned long highestScore = 0;
  int highScorePlayerNum = 0;
  for (int count = 0; count < CurrentNumPlayers; count++) {
    if (CurrentScores[count] > highestScore) highestScore = CurrentScores[count];
    highScorePlayerNum = count;
  }

  if (highestScore > HighScore) {
    HighScore = highestScore;
    if (HighScoreReplay) {
      AddCredit(false, 3);
      RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 3);
    }
    RPU_WriteULToEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, highestScore);
    RPU_WriteULToEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE) + 1);

    for (int count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
      if (count == highScorePlayerNum) {
        RPU_SetDisplay(count, CurrentScores[count], true, 2);
      } else {
        RPU_SetDisplayBlank(count, 0x00);
      }
    }

    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, CurrentTime, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, CurrentTime + 300, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, CurrentTime + 600, true);
  }
}


unsigned long MatchSequenceStartTime = 0;
unsigned long MatchDelay = 150;
byte MatchDigit = 0;
byte NumMatchSpins = 0;
byte ScoreMatches = 0;

int ShowMatchSequence(boolean curStateChanged) {
  if (!MatchFeature) return MACHINE_STATE_ATTRACT;

  if (curStateChanged) {
    MatchSequenceStartTime = CurrentTime;
    MatchDelay = 1500;
    MatchDigit = CurrentTime % 10;
    NumMatchSpins = 0;
    RPU_SetLampState(LAMP_HEAD_MATCH, 1, 0);
    RPU_SetDisableFlippers();
    ScoreMatches = 0;
  }

  if (NumMatchSpins < 40) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      MatchDigit += 1;
      if (MatchDigit > 9) MatchDigit = 0;
      //PlaySoundEffect(10+(MatchDigit%2));
      PlaySoundEffect(SOUND_EFFECT_MATCH_SPIN);
      RPU_SetDisplayBallInPlay((int)MatchDigit * 10);
      MatchDelay += 50 + 4 * NumMatchSpins;
      NumMatchSpins += 1;
      RPU_SetLampState(LAMP_HEAD_MATCH, NumMatchSpins % 2, 0);

      if (NumMatchSpins == 40) {
        RPU_SetLampState(LAMP_HEAD_MATCH, 0);
        MatchDelay = CurrentTime - MatchSequenceStartTime;
      }
    }
  }

  if (NumMatchSpins >= 40 && NumMatchSpins <= 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      if ( (CurrentNumPlayers > (NumMatchSpins - 40)) && ((CurrentScores[NumMatchSpins - 40] / 10) % 10) == MatchDigit) {
        ScoreMatches |= (1 << (NumMatchSpins - 40));
        AddSpecialCredit();
        MatchDelay += 1000;
        NumMatchSpins += 1;
        RPU_SetLampState(LAMP_HEAD_MATCH, 1);
      } else {
        NumMatchSpins += 1;
      }
      if (NumMatchSpins == 44) {
        MatchDelay += 5000;
      }
    }
  }

  if (NumMatchSpins > 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      return MACHINE_STATE_ATTRACT;
    }
  }

  for (int count = 0; count < 4; count++) {
    if ((ScoreMatches >> count) & 0x01) {
      // If this score matches, we're going to flash the last two digits
      byte upperMask = 0x0F;
      byte lowerMask = 0x30;
      if (RPU_OS_NUM_DIGITS == 7) {
        upperMask = 0x1F;
        lowerMask = 0x60;
      }
      if ( (CurrentTime / 200) % 2 ) {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) & upperMask);
      } else {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) | lowerMask);
      }
    }
  }

  return MACHINE_STATE_MATCH_MODE;
}




////////////////////////////////////////////////////////////////////////////
//
//  Switch Handling functions
//
////////////////////////////////////////////////////////////////////////////

int HandleSystemSwitches(int curState, byte switchHit) {
  int returnState = curState;
  switch (switchHit) {
    case SW_SELF_TEST_SWITCH:
      Menus.EnterOperatorMenu();
      break;
    case SW_COIN_1:
    case SW_COIN_2:
    case SW_COIN_3:
      {
        byte chuteNum = SwitchToChuteNum(switchHit);
        if ((CurrentTime-LastCoinSwitchTime[chuteNum])>250) {
          LastCoinSwitchTime[chuteNum] = CurrentTime;
          AddCoinToAudit(chuteNum);
          AddCoin(chuteNum);
        }
      }
      break;
    case SW_CREDIT_RESET:
      if (MachineState == MACHINE_STATE_MATCH_MODE) {
        // If the first ball is over, pressing start again resets the game
        if (Credits >= 1 || FreePlayMode) {
          if (!FreePlayMode) {
            Credits -= 1;
            RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
            RPU_SetDisplayCredits(Credits, !FreePlayMode);
          }
          returnState = MACHINE_STATE_INIT_GAMEPLAY;
        }
      } else {
        CreditResetPressStarted = CurrentTime;
      }
      break;
    case SW_OUTHOLE:
      // Some machines have a kicker to move the ball
      // from the outhole to the re-shooter ramp
      break;
    case SW_PLUMB_TILT:
      if (BallFirstSwitchHitTime) {
        if ( CurrentTime > (LastTiltWarningTime + TILT_WARNING_DEBOUNCE_TIME) ) {
          LastTiltWarningTime = CurrentTime;
          NumTiltWarnings += 1;
          if (NumTiltWarnings > MaxTiltWarnings) {
            RPU_DisableSolenoidStack();
            RPU_SetDisableFlippers(true);
            RPU_TurnOffAllLamps();
            Audio.StopAllAudio();
            if (BallSaveEndTime) {
              BallSaveEndTime = 0;
              NumberOfBallSavesRemaining = 0;
            }
            RPU_SetLampState(LAMP_HEAD_TILT, 1);
            PlaySoundEffect(SOUND_EFFECT_TILT);
          } else {
            PlaySoundEffect(SOUND_EFFECT_TILT_WARNING);
          }
        }
      } else {
        // Tilt before ball is plunged -- show a timer in ManageGameMode if desired
        if ( CurrentTime > (LastTiltWarningTime + TILT_WARNING_DEBOUNCE_TIME) ) {
          PlaySoundEffect(SOUND_EFFECT_TILT_WARNING);
        }
        LastTiltWarningTime = CurrentTime;
      }
      break;
  }

  return returnState;
}



boolean LockBall(byte saucerIndex) {
  if ( (PlayerLocks[CurrentPlayer]&(BALL_LEFT_LOCK_QUALIFIED<<saucerIndex))==0 ) return false;
  NumberOfBallsLocked += 1;
  MachineLocks |= (BALL_LEFT_LOCK_ENGAGED<<saucerIndex);
  PlayerLocks[CurrentPlayer] |= (BALL_LEFT_LOCK_ENGAGED<<saucerIndex);
  PlayerLocks[CurrentPlayer] &= ~(BALL_LEFT_LOCK_QUALIFIED<<saucerIndex);
  return true;
}


boolean ReleaseOtherPlayersLock(byte lockNotToRelease) {
  for (byte count=0; count<3; count++) {
    if (count!=lockNotToRelease) {
      // Check to see if this Machine lock is set
      if (MachineLocks & (BALL_LEFT_LOCK_ENGAGED<<count)) {
        // Make sure this lock doesn't belong to this player
        if ( (PlayerLocks[CurrentPlayer] & (BALL_LEFT_LOCK_ENGAGED<<count))==0 ) {
          // We can release this lock
          if (NumberOfBallsLocked) NumberOfBallsLocked -= 1;
          MachineLocks &= ~(BALL_LEFT_LOCK_ENGAGED<<count);
          RPU_PushToSolenoidStack(SaucerSolenoids[count], SaucerSolenoidStrength, true);
          return true;
        }
      }
    }
  }

  return false;
}


void SwapLocks(byte flag1, byte flag2) {
  // This probably doesn't need a whole function, but 
  // if I ever decide to move weapons stuff with each
  // ship then it could come in handy.

  PlayerLocks[CurrentPlayer] &= ~flag1;
  PlayerLocks[CurrentPlayer] |= flag2;

  // move qualified flag if necessary
  if ( (PlayerLocks[CurrentPlayer] & (flag2/16)) ) {
    PlayerLocks[CurrentPlayer] |= (flag1/16);
  }
}


/*
 * UpdatePlayerLocks() Algorithm:
 * 1) Update Machine locks based on saucer switches (just in case)
 * 2) Look for player locks engaged that don't exist in machine locks
 *    a) if found, try to relocate the player lock to a machine lock that doesn't have a player lock
 *        (relocate means swap engaged flags and qualified flags)
 *    b) if can't relocate (there are no machine locks without player locks), then downgrade to 
 *        lock qualified
 * 3) Look for player locks qualified that are currently blocked by machine locks
 *    a) if blocked, then try to move qualified lock to spot where there's no machine lock
 * 
 */


void UpdatePlayerLocks() {
  // At the beginning of a ball, we have to review our 
  // player locks versus the machine locks and saucers to see if
  // anything needs to be updated

  // 1) reset machine locks to match saucers
  byte lockEngagedFlag = BALL_LEFT_LOCK_ENGAGED;
  MachineLocks = 0;
  for (byte count=0; count<3; count++) {
    boolean saucerSwitchDown = RPU_ReadSingleSwitchState(SaucerSwitches[count]);
    if (saucerSwitchDown) MachineLocks |= lockEngagedFlag;
    lockEngagedFlag *= 2;
  }

  // 2) relocate any orphaned player locks (if possible)
  lockEngagedFlag = BALL_LEFT_LOCK_ENGAGED;
  for (byte count=0; count<3; count++) {
    // An orphaned lock is one where we have 
    // PlayerLocks set to ENGAGED, but no corresponding
    // MachineLocks
    if ( (PlayerLocks[CurrentPlayer]&lockEngagedFlag) && (MachineLocks&lockEngagedFlag)==0 ) {
      // This is an orphaned lock, our preference is to swap it
      // with a MachineLock that's not currently associated with 
      // a PlayerLock
      boolean lockSwapped = false;
      for (byte mLockTest=1; mLockTest<3; mLockTest++) {
        byte mLockFlag = BALL_LEFT_LOCK_ENGAGED << ((mLockTest+count)%3);
        if ( (MachineLocks&mLockFlag) && (PlayerLocks[CurrentPlayer]&mLockFlag)==0 ) {
          // this lock appears in MachineLocks but not in 
          // this player's locks, so we can swap it
          SwapLocks(lockEngagedFlag, mLockFlag);
          lockSwapped = true;
          break;
        }
      }
      if (!lockSwapped) {
        // we were unable to re-home the orphaned lock, so
        // now we need to downgrade it to qualified
        PlayerLocks[CurrentPlayer] &= ~(lockEngagedFlag);
        PlayerLocks[CurrentPlayer] |= (lockEngagedFlag/16);
      }
      
    }
    lockEngagedFlag *= 2;
  }

  // 3) Relocate any qualified locks that are pointed to filled MachineLocks
  //      (if possible) 

  byte lockQualifiedFlag = BALL_LEFT_LOCK_QUALIFIED;
  for (byte count=0; count<3; count++) {
    // Look for qualified player lock that's
    // currently filled
    if ( (PlayerLocks[CurrentPlayer]&lockQualifiedFlag) && (MachineLocks&(lockQualifiedFlag*16)) ) {
      for (byte mLockTest=1; mLockTest<3; mLockTest++) {
        byte mLockFlag = BALL_LEFT_LOCK_ENGAGED << ((mLockTest+count)%3);
        if ( (MachineLocks&mLockFlag)==0 ) {
          // this MachineLock is not taken, so we can put the qualified here
          SwapLocks(lockQualifiedFlag, mLockFlag/16);
        }
      }      
    }
    lockQualifiedFlag *= 2;
  }
}


byte GetNextLockToQualify(boolean startAtTop) {
  byte lockToQualify = BALL_LEFT_LOCK_ENGAGED;
  if (startAtTop) lockToQualify = BALL_TOP_LOCK_ENGAGED;

  byte count;
  for (count=0; count<3; count++) {
    if ( (MachineLocks & lockToQualify)==0  && (PlayerLocks[CurrentPlayer] & (lockToQualify/16))==0 ) {
      break;
    }
    lockToQualify *= 2;
    if (lockToQualify>BALL_RIGHT_LOCK_ENGAGED) lockToQualify = BALL_LEFT_LOCK_ENGAGED;
  }
  if (count==3) {
    // we have to re-use a lock that's already machine locked
    // meaning that we'll have to kick a ball when a lock is taken
    lockToQualify = BALL_LEFT_LOCK_ENGAGED;
    if (startAtTop) lockToQualify = BALL_TOP_LOCK_ENGAGED;
    for (count=0; count<3; count++) {
      if ( (MachineLocks & lockToQualify) && (PlayerLocks[CurrentPlayer] & (lockToQualify/16))==0 ) {
        break;
      }
      lockToQualify *= 2;
      if (lockToQualify>BALL_RIGHT_LOCK_ENGAGED) lockToQualify = BALL_LEFT_LOCK_ENGAGED;
    }

    if (count<3) {
      lockToQualify /= 16;
      return lockToQualify;
    }
    return 0;
  }
  lockToQualify /= 16;
  return lockToQualify;
}


boolean QualifyNextLock(boolean startAtTop) {
  byte nextLock = GetNextLockToQualify(startAtTop);

  if (nextLock) {
    byte locksBefore = PlayerLocks[CurrentPlayer] & BALL_LOCKS_QUALIFIED;
    PlayerLocks[CurrentPlayer] |= nextLock;

    if (locksBefore) {
      if (nextLock==BALL_LEFT_LOCK_QUALIFIED) QueueNotification(SOUND_EFFECT_VP_LEFT_DOCK_READY, 4);
      else if (nextLock==BALL_TOP_LOCK_QUALIFIED) QueueNotification(SOUND_EFFECT_VP_TOP_DOCK_READY, 4);
      else QueueNotification(SOUND_EFFECT_VP_RIGHT_DOCK_READY, 4);
    } else {
      if (nextLock==BALL_LEFT_LOCK_QUALIFIED) QueueNotification(SOUND_EFFECT_VP_LEFT_DOCK_READY_LONG, 4);
      else if (nextLock==BALL_TOP_LOCK_QUALIFIED) QueueNotification(SOUND_EFFECT_VP_TOP_DOCK_READY_LONG, 4);
      else QueueNotification(SOUND_EFFECT_VP_RIGHT_DOCK_READY_LONG, 4);
    }
    
    return true;
  }

  return false;
}


void QualifyPlayerTraining(byte trainingMode) {

  if (trainingMode==FLIGHT_TRAINING) trainingMode = FLIGHT_TRAINING_QUALIFIED;
  if (trainingMode==WEAPONS_TRAINING) trainingMode = WEAPONS_TRAINING_QUALIFIED;

  if (PlayerTrainingStatus[CurrentPlayer]&trainingMode) {
    // this training is already qualified, so nothing to do
    return;
  }

  PlayerTrainingStatus[CurrentPlayer] |= trainingMode;

  if (trainingMode==WEAPONS_TRAINING_QUALIFIED) {
    QueueNotification(SOUND_EFFECT_VP_WEAPONS_TRAINING_QUALIFIED, 7);
  } else if (trainingMode==FLIGHT_TRAINING_QUALIFIED) {
    QueueNotification(SOUND_EFFECT_VP_FLIGHT_TRAINING_QUALIFIED, 7);
  }
#ifdef DEBUG_MESSAGES
  char buf[128];
  sprintf(buf, "Player Training = 0x%02X\n", PlayerTrainingStatus[CurrentPlayer]);
  Serial.write(buf);
#endif  

/*  
  byte otherTrainingRunning = (PlayerTrainingStatus[CurrentPlayer]&TRAINING_RUNNING_MASK) & (~trainingMode);

  switch (trainingMode) {
    case WEAPONS_TRAINING:
      if (PlayerTrainingStatus[CurrentPlayer] & WEAPONS_TRAINING_ACHIEVED_MASK) {
        // this training has been completed successfully before, so announce
        // that it's a repeat
        QueueNotification(SOUND_EFFECT_VP_REFRESHER_WEAPONS_TRAINING, 7);
      } else if (otherTrainingRunning) {
        // There is another training happening, so announce this as
        // an addition
        QueueNotification(SOUND_EFFECT_VP_ADDING_WEAPONS_TRAINING, 7);
      } else {
        // This is the start of a training mission
        QueueNotification(SOUND_EFFECT_VP_STARTING_WEAPONS_TRAINING, 7);
      }     
      WeaponsTrainingEndTime = CurrentTime + ((unsigned long)TrainingDuration * 1000);
      IncreasePlayfieldMultiplier(((unsigned long)TrainingDuration + 10) * 1000);
      PopHits[CurrentPlayer] = 0;
      break;

    case FLIGHT_TRAINING:
      if (PlayerTrainingStatus[CurrentPlayer] & FLIGHT_TRAINING_ACHIEVED) {
        // this training has been completed successfully before, so announce
        // that it's a repeat
        QueueNotification(SOUND_EFFECT_VP_REFRESHER_FLIGHT_TRAINING, 7);
      } else if (otherTrainingRunning) {
        // There is another training happening, so announce this as
        // an addition
        QueueNotification(SOUND_EFFECT_VP_ADDING_FLIGHT_TRAINING, 7);
      } else {
        // This is the start of a training mission
        QueueNotification(SOUND_EFFECT_VP_STARTING_FLIGHT_TRAINING, 7);
      }
      FlightTrainingEndTime = CurrentTime + ((unsigned long)TrainingDuration * 1000);
      IncreasePlayfieldMultiplier(((unsigned long)TrainingDuration + 10) * 1000);
      SpinnerHits[CurrentPlayer] = 0;// We'll use this to track spins during training
      break;

  }
*/  
}


boolean StartPlayerTraining(byte saucerNum) {
  // If there's no training qualified, return false to move on with 
  // unstructured play
  if (GameMode!=GAME_MODE_UNSTRUCTURED_PLAY) return false;
  if ((PlayerTrainingStatus[CurrentPlayer] & TRAINING_QUALIFIED_MASK)==0) return false;  

  OfferBattleSaucer = saucerNum;
  SetGameMode(GAME_MODE_START_TRAINING);
  
  return true;
}


void CompletePlayerTraining(byte trainingMode) {
  
  Display_StartScoreAnimation( ((unsigned long)TrainingBonus * 1000) * PlayfieldMultiplier, true);
  AdvanceNickname(CurrentPlayer);
  PlayerRank[CurrentPlayer] += 1;
  if (PlayerRank[CurrentPlayer]>4) PlayerRank[CurrentPlayer] = 4;
  CurrentScores[CurrentPlayer] -= (CurrentScores[CurrentPlayer]%10);
  CurrentScores[CurrentPlayer] += PlayerRank[CurrentPlayer];

  byte currentTrainingLevel = 0;
  switch (trainingMode) {
    case WEAPONS_TRAINING:
      PlayerTrainingStatus[CurrentPlayer] &= ~(WEAPONS_TRAINING_QUALIFIED | WEAPONS_TRAINING_RUNNING);
      QueueNotification(SOUND_EFFECT_VP_WEAPONS_TRAINING_COMPLETE, 7);
      currentTrainingLevel = (PlayerTrainingStatus[CurrentPlayer] & WEAPONS_TRAINING_ACHIEVED_MASK) / WEAPONS_TRAINING_ACHIEVED_SHIFT;
      if (currentTrainingLevel<3) {
        currentTrainingLevel += 1;
        PlayerTrainingStatus[CurrentPlayer] &= ~WEAPONS_TRAINING_ACHIEVED_MASK;
        PlayerTrainingStatus[CurrentPlayer] |= (currentTrainingLevel * WEAPONS_TRAINING_ACHIEVED_SHIFT);
      }
      PopHits[CurrentPlayer] = 0;
      break;
    case FLIGHT_TRAINING:
      PlayerTrainingStatus[CurrentPlayer] &= ~(FLIGHT_TRAINING_QUALIFIED | FLIGHT_TRAINING_RUNNING);
      QueueNotification(SOUND_EFFECT_VP_FLIGHT_TRAINING_COMPLETE, 7);
      currentTrainingLevel = (PlayerTrainingStatus[CurrentPlayer] & FLIGHT_TRAINING_ACHIEVED_MASK) / FLIGHT_TRAINING_ACHIEVED_SHIFT;
      if (currentTrainingLevel<3) {
        currentTrainingLevel += 1;
        PlayerTrainingStatus[CurrentPlayer] &= ~FLIGHT_TRAINING_ACHIEVED_MASK;
        PlayerTrainingStatus[CurrentPlayer] |= (currentTrainingLevel * FLIGHT_TRAINING_ACHIEVED_SHIFT);
      }
      SpinnerHits[CurrentPlayer] = 0;
      break;
  }

  QueueNotification(SOUND_EFFECT_VP_RANK_INCREASED, 7);
  QueueNotification(SOUND_EFFECT_VP_YOUR_RANK_IS + (CurrentTime%3), 7);
  QueueNotification(GetRankSoundIndex(PlayerNickname[CurrentPlayer]), 7);
    
}


unsigned long LastBullseyeHit = 0;

void HandleBullseye() {
  if (LastBullseyeHit && CurrentTime<(LastBullseyeHit+250)) return;
  LastBullseyeHit = CurrentTime;
  SwitchHitsInMode += 1;
  
  if (BattleStage==BATTLE_STAGE_BULLSEYE) {
    if (BattleStageShots) BattleStageShots -= 1;
    PlaySoundEffect(SOUND_EFFECT_BATTLE_BULLSEYE);
    LastTimeBattleShotHit = CurrentTime;
  } else {
    PlaySoundEffect(SOUND_EFFECT_BULLSEYE);
    if (FirePowerLevelChangedTime) {
      GiveFirePowerAward();
    } else {
      CurrentScores[CurrentPlayer] += 1000 * PlayfieldMultiplier;
    }
  }
}


void HandleSpinnerProgress() {

  LastTimeFlightTrainingHit = CurrentTime;
  LastTrainingHitTime = CurrentTime;
  boolean soundPlayed = false;
  
  if (BattleStage==BATTLE_STAGE_SPINNER) {
    if (BattleStageShots>=(ShipThrusters[CurrentPlayer]+1)) BattleStageShots -= (ShipThrusters[CurrentPlayer]+1);
    else BattleStageShots = 0;
    Audio.StopSound(SOUND_EFFECT_BATTLE_SPINNER);
    PlaySoundEffect(SOUND_EFFECT_BATTLE_SPINNER);
    LastTimeBattleShotHit = CurrentTime;
    soundPlayed = true;
  }

  if (PlayerTrainingStatus[CurrentPlayer] & FLIGHT_TRAINING_RUNNING) {
    // We're in flight training, so we will award flight training
    // points and track spins to the goal
    FlightTrainingHits[CurrentPlayer] += 1;
    if (FlightTrainingHits[CurrentPlayer]>=100) FlightTrainingHits[CurrentPlayer] = 99;
    // scoring while reaching goal
    CurrentScores[CurrentPlayer] += (1250 * PlayfieldMultiplier);
    if (!soundPlayed) {
      Audio.StopSound(SOUND_EFFECT_SPINNER_LIT);
      PlaySoundEffect(SOUND_EFFECT_SPINNER_LIT);
    }    
  } else {
    if (  (PlayerTrainingStatus[CurrentPlayer] & FLIGHT_TRAINING_QUALIFIED)==0 && 
          GameMode==GAME_MODE_UNSTRUCTURED_PLAY && 
          SpinnerHits[CurrentPlayer]<CalculateShotsNeededForNextTraining(FLIGHT_TRAINING) ) {
      SpinnerHits[CurrentPlayer] += 1;
      if (SpinnerHits[CurrentPlayer]>=CalculateShotsNeededForNextTraining(FLIGHT_TRAINING)) {
        QualifyPlayerTraining(FLIGHT_TRAINING);
      }
    }

    if (RightInlaneStage) {
      CurrentScores[CurrentPlayer] += (1000 * PlayfieldMultiplier * (unsigned long)RightInlaneStage);
      if (!soundPlayed) {
        Audio.StopSound(SOUND_EFFECT_SPINNER_LIT);
        PlaySoundEffect(SOUND_EFFECT_SPINNER_LIT);
      }
    } else {
      unsigned long spinnerPts = 100;
      if (FlightTrainingHits[CurrentPlayer]) {
        spinnerPts += (unsigned long)FlightTrainingHits[CurrentPlayer] * 50;
      }
      CurrentScores[CurrentPlayer] += (spinnerPts * PlayfieldMultiplier);
      if (!soundPlayed) {
        Audio.StopSound(SOUND_EFFECT_SPINNER_UNLIT);
        PlaySoundEffect(SOUND_EFFECT_SPINNER_UNLIT);
      }
    }
  }

}


void ValidateAndRegisterPlayfieldSwitch() {
  LastSwitchHitTime = CurrentTime;
  if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
}


void AddBossBattleHit() {
  if (GameMode!=GAME_MODE_BOSS_BATTLE) return; // don't know how we got here, but leave

  byte stageIndex = BossBattleStage - 1;
  BossHitsInStage[stageIndex] += 1;

  byte stageMask = 0x01 << stageIndex;

  if (BossBattleStagesDone & stageMask) {
    // We've already finished this stage, so we get a higher award
    BossBattleBonus += 10000;
    PlaySoundEffect(SOUND_EFFECT_FIRE_COMPLETED);
  } else {
    DetermineIfBossStageComplete();
    BossBattleBonus += 5000;
    PlaySoundEffect(SOUND_EFFECT_MULTIBALL_PULSE);
  }

  if (DEBUG_MESSAGES) {
    char buf[256];
    sprintf(buf, "F=%d P=%d N=%d P=%d\n", BossHitsInStage[0], BossHitsInStage[1], BossHitsInStage[2], BossHitsInStage[3]);
    Serial.write(buf);
  }
}


void HandleStandupHit(byte switchNum) {

  byte standupID = switchNum - SW_1_STANDUP;
  if (switchNum>=SW_4_STANDUP) {
    standupID = switchNum - SW_4_STANDUP;
  }
  if (CurrentTime<(LastTimeStandupSeen[standupID]+250)) return;
  LastTimeStandupSeen[standupID] = CurrentTime;
  SwitchHitsInMode += 1;
  
  if (BattleStage!=BATTLE_STAGE_OFF) {
    if (BattleStage==BATTLE_STAGE_STANDUPS) {
      byte switchIndex = switchNum-SW_1_STANDUP;
      if (switchNum>SW_3_STANDUP) switchIndex = (switchNum-SW_4_STANDUP) + 3;
      byte switchBit = 0x01 << switchIndex;
  
      if (switchBit & BattleStandups) {
        //BattleStandups &= ~(switchBit);
        PlaySoundEffect(SOUND_EFFECT_BATTLE_STANDUP_HIT);
        LastBattleStandupHitTime = CurrentTime;
        LastBattleStandupHit = switchIndex;
        if (BattleStageShots) BattleStageShots -= 1;
        LastTimeBattleShotHit = CurrentTime;
      } else {
        CurrentScores[CurrentPlayer] += 500 * PlayfieldMultiplier;
        PlaySoundEffect(SOUND_EFFECT_STANDUP_REHIT);
      }
      return;
    } else {
      CurrentScores[CurrentPlayer] += 5000 * PlayfieldMultiplier;
      PlaySoundEffect(SOUND_EFFECT_10PT_REHIT);
      return;
    }
  } else if (GameMode==GAME_MODE_BOSS_BATTLE) {
    if (BossBattleStage==BOSS_STAGE_NUMBER_STANDUPS) {
      AddBossBattleHit();
    } else {
      PlaySoundEffect(SOUND_EFFECT_STANDUP_REHIT);  
    }
  } else {
  
    if (switchNum==SW_1_STANDUP || switchNum==SW_2_STANDUP || switchNum==SW_3_STANDUP) {
      byte switchBit = 0x01;
      if (switchNum>SW_1_STANDUP) switchBit = 0x01<<(switchNum-SW_1_STANDUP);
      if ((StandupTargetStatus[CurrentPlayer]&switchBit)==0) {
        // This is a new hit
        // Award points and qualify lock (when appropriate)
        StandupTargetStatus[CurrentPlayer] |= switchBit;
        CurrentScores[CurrentPlayer] += 1000 * PlayfieldMultiplier;
        if ((StandupTargetStatus[CurrentPlayer]&0x07)==0x07) {
          // Bank is finished
          CurrentScores[CurrentPlayer] += ((unsigned long)StandupTargetCompletions[CurrentPlayer] + 1) * 5000 * PlayfieldMultiplier;
          if (BallKickerBehavior==BALL_KICKER_EASY || BallKickerBehavior==BALL_KICKER_MEDIUM) LeftKickback[CurrentPlayer] = 1;
          
          if (StandupTargetStatus[CurrentPlayer]==0x77) {
            if (BallKickerBehavior==BALL_KICKER_HARD) LeftKickback[CurrentPlayer] = 1;
            StandupTargetCompletions[CurrentPlayer] += 1;
            StandupTargetStatus[CurrentPlayer] = 0;
            StandupTargetFinishTime = CurrentTime;
            QualifyNextLock(false);
            PlaySoundEffect(SOUND_EFFECT_ALL_STANDUPS_COMPLETED);
            StandupTargetStatus[CurrentPlayer] = 0x00;
          } else {
            if (StandupTargetCompletions[CurrentPlayer]==0 && SingleBankQualifiesFirstTwoLocks) QualifyNextLock(false);
            PlaySoundEffect(SOUND_EFFECT_STANDUP_BANK_COMPLETED);
          }
        } else {
          // This is a new target hit, but not a completion
          PlaySoundEffect(SOUND_EFFECT_STANDUP_HIT);
        }
      } else {
        // This is a re-hit of a lit switch  
        CurrentScores[CurrentPlayer] += 500 * PlayfieldMultiplier;
        PlaySoundEffect(SOUND_EFFECT_STANDUP_REHIT);
      }
  
    }
  
  
    if (switchNum==SW_4_STANDUP || switchNum==SW_5_STANDUP || switchNum==SW_6_STANDUP) {
      byte switchBit = 0x10;
      if (switchNum>SW_4_STANDUP) switchBit = 0x10<<(switchNum-SW_4_STANDUP);
      if ((StandupTargetStatus[CurrentPlayer]&switchBit)==0) {
        // This is a new hit
        // Award points and qualify lock (when appropriate)
        StandupTargetStatus[CurrentPlayer] |= switchBit;
        CurrentScores[CurrentPlayer] += 1000 * PlayfieldMultiplier;
        if ((StandupTargetStatus[CurrentPlayer]&0x70)==0x70) {
          // Bank is finished
          CurrentScores[CurrentPlayer] += ((unsigned long)StandupTargetCompletions[CurrentPlayer] + 1) * 5000 * PlayfieldMultiplier;
          if (BallKickerBehavior==BALL_KICKER_EASY || BallKickerBehavior==BALL_KICKER_MEDIUM) LeftKickback[CurrentPlayer] = 1;
  
          if (StandupTargetStatus[CurrentPlayer]==0x77) {
            if (BallKickerBehavior==BALL_KICKER_HARD) LeftKickback[CurrentPlayer] = 1;
            StandupTargetCompletions[CurrentPlayer] += 1;
            StandupTargetStatus[CurrentPlayer] = 0;
            StandupTargetFinishTime = CurrentTime;
            QualifyNextLock(false);
            PlaySoundEffect(SOUND_EFFECT_ALL_STANDUPS_COMPLETED);
            StandupTargetStatus[CurrentPlayer] = 0x00;
          } else {
            if (StandupTargetCompletions[CurrentPlayer]==0 && SingleBankQualifiesFirstTwoLocks) QualifyNextLock(false);
            PlaySoundEffect(SOUND_EFFECT_STANDUP_BANK_COMPLETED);
          }               
        } else {
          // This is a new target hit, but not a completion
          PlaySoundEffect(SOUND_EFFECT_STANDUP_HIT);
        }
      } else {
        // This is a re-hit of a lit switch  
        CurrentScores[CurrentPlayer] += 500 * PlayfieldMultiplier;
        PlaySoundEffect(SOUND_EFFECT_STANDUP_REHIT);
      }
    }
  }
}


void OfferLockOrBattle(byte saucerNum) {
  // Logic for battles & locks:
  //  If you haven't attempted any battles, you can only attempt 2 or 3
  //  If you have attempted a battle, you can then attempt 1
  //  If you've attempted a 1 and a 2, you have to attempt a 3 before you can do them again
  byte countBits = CountBits(PlayerLocks[CurrentPlayer] & BALL_LOCKS_ENGAGED);
  OfferBattleSaucer = saucerNum;
  if (countBits==0) {
    if (BattlesPlayed[CurrentPlayer]&0x04) {
      // 4, 5, 6, 7
      SetGameMode(GAME_MODE_OFFER_BATTLE_1);      
    } else if (BattlesPlayed[CurrentPlayer]==0x02) {
      // 2
      SetGameMode(GAME_MODE_OFFER_BATTLE_1);
    } else {
      // 0, 1, 3
      LockInsteadOfBattle();
    }
  } else if (countBits==1) {
    if (BattlesPlayed[CurrentPlayer]&0x04) {
      // 4, 5, 6, 7
      SetGameMode(GAME_MODE_OFFER_BATTLE_2);
    } else if (!(BattlesPlayed[CurrentPlayer]&0x01)) {
      // 0, 2
      SetGameMode(GAME_MODE_OFFER_BATTLE_2);
    } else {
      // 1, 3
      LockInsteadOfBattle();
    }
  } else if (countBits==2) {
    SetGameMode(GAME_MODE_START_BATTLE_3);
  }
  
}


void HandleSaucer(byte saucerNum) {

  // If this lock is already filled (for any player)
  // then this is a "knocked" ball and we should ignore this lock
  if (MachineLocks&(BALL_LEFT_LOCK_ENGAGED << saucerNum)) return;

  ValidateAndRegisterPlayfieldSwitch();
  if (0 && GameMode==GAME_MODE_SKILL_SHOT) {
    ValidateAndRegisterPlayfieldSwitch();
    // Give an award for a saucer skillshot 
    byte qualifiedBit = BALL_LEFT_LOCK_QUALIFIED<<saucerNum;
    byte lockBit = BALL_LEFT_LOCK_ENGAGED<<saucerNum;
    if ( (PlayerLocks[CurrentPlayer] & lockBit) || (MachineLocks & lockBit) ) {
      // This can't be true, can it?
      PlayerLocks[CurrentPlayer] &= ~lockBit;
      MachineLocks &= ~lockBit;
      RPU_PushToSolenoidStack(SaucerSolenoids[saucerNum], SaucerSolenoidStrength, true);
    } else {
      if (PlayerLocks[CurrentPlayer] & qualifiedBit) {
        // Offer a ball lock or battle
        OfferLockOrBattle(saucerNum);
      } else {
        if (SkillShotQualifiesLock) {
          PlayerLocks[CurrentPlayer] |= qualifiedBit;
        }
        // Add score and play a notification
        QueueNotification(SOUND_EFFECT_VP_SUPER_SKILL_SHOT_1 + CurrentTime%3, 5);
        Display_StartScoreAnimation(SKILL_SHOT_REWARD * ((unsigned long)2), true);
        RPU_PushToSolenoidStack(SaucerSolenoids[saucerNum], SaucerSolenoidStrength, true);
      }
    }
    RPU_PushToSolenoidStack(SaucerSolenoids[saucerNum], SaucerSolenoidStrength, true);
  } else if (GameMode==GAME_MODE_UNSTRUCTURED_PLAY || GameMode==GAME_MODE_SKILL_SHOT) {
    byte qualifiedBit = BALL_LEFT_LOCK_QUALIFIED<<saucerNum;
    if (PlayerLocks[CurrentPlayer] & qualifiedBit) {
      // Offer a ball lock or battle
      OfferLockOrBattle(saucerNum);
    } else {
      // If training is qualified, we can start it here
      if (!StartPlayerTraining(saucerNum)) RPU_PushToSolenoidStack(SaucerSolenoids[saucerNum], SaucerSolenoidStrength, true);
    }    
  } else if (GameMode==GAME_MODE_START_TRAINING) {
    // Don't need to do anything if the training is starting
    // because eject will be handled by the ManageGameMode function
  } else if (GameMode==GAME_MODE_BOSS_BATTLE) {
    BossSaucerKickoutTime[saucerNum] = CurrentTime + BOSS_BATTLE_SAUCER_HOLD_TIME;
  } else {
    if (BattleStage==BATTLE_STAGE_SAUCER && saucerNum==OfferBattleSaucer) {
      if (BattleStageShots) BattleStageShots -= 1;
      LastTimeBattleShotHit = CurrentTime;
      if (BattleStageShots==0) {
        // the battle is finished, so we'll let GAME_MODE_BATTLE_1_WON kick the ball
      } else {
        // Battle not finished yet
        RPU_PushToSolenoidStack(SaucerSolenoids[saucerNum], SaucerSolenoidStrength, true);
      }
    } else if (GameMode==GAME_MODE_BATTLE_1 && saucerNum==OfferBattleSaucer) {
      BattleRefuelingTime = CurrentTime;
      Display_ClearOverride(0xFF);
      if (!RefuelingMessagePlayed) {
        RefuelingMessagePlayed = true;
        QueueNotification(SOUND_EFFECT_VP_REFUELING, 5);
      }
    } else if (JackpotReady) {
      JackpotReady = false;
      switch (NumberOfBallsInPlay) {
        case 1:
          Display_StartScoreAnimation( ((unsigned long)JackpotValue[CurrentPlayer] * ((unsigned long)PlayerRank[CurrentPlayer]+1) * 1000) * PlayfieldMultiplier, true);
          QueueNotification(SOUND_EFFECT_VP_JACKPOT_1 + CurrentTime%6, 5);
          break;
        case 2:
          Display_StartScoreAnimation( ((unsigned long)JackpotValue[CurrentPlayer] * ((unsigned long)PlayerRank[CurrentPlayer]+1) * 2000) * PlayfieldMultiplier, true);
          QueueNotification(SOUND_EFFECT_VP_DOUBLE_JACKPOT, 5);
          break;
        case 3:
          Display_StartScoreAnimation( ((unsigned long)JackpotValue[CurrentPlayer] * ((unsigned long)PlayerRank[CurrentPlayer]+1) * 3000) * PlayfieldMultiplier, true);
          QueueNotification(SOUND_EFFECT_VP_TRIPLE_JACKPOT, 5);
          break;
      }
      RPU_PushToTimedSolenoidStack(SaucerSolenoids[saucerNum], SaucerSolenoidStrength, CurrentTime+5000, true);
    } else {
      if (GameMode==GAME_MODE_BATTLE_2 || GameMode==GAME_MODE_BATTLE_3) {
        RPU_PushToTimedSolenoidStack(SaucerSolenoids[saucerNum], SaucerSolenoidStrength, CurrentTime+7500, true);
      } else {
        RPU_PushToSolenoidStack(SaucerSolenoids[saucerNum], SaucerSolenoidStrength, true);
      }
    }
  }
}


void GiveFirePowerAward() {
  switch (FirePowerLevel[CurrentPlayer]) {
    case 0:
      CurrentScores[CurrentPlayer] += (1000 * PlayfieldMultiplier);
      break;
    case 1:
      Display_StartScoreAnimation(10000 * PlayfieldMultiplier, true);
      break;
    case 2:
      Display_StartScoreAnimation(30000 * PlayfieldMultiplier, true);
      break;
    case 3:
      Display_StartScoreAnimation(50000 * PlayfieldMultiplier, true);
      break;
    case 4:
      Display_StartScoreAnimation(65000 * PlayfieldMultiplier, true);
      break;
    case 5:
      Display_StartScoreAnimation(75000 * PlayfieldMultiplier, true);
      break;
    default:
      Display_StartScoreAnimation(100000 * PlayfieldMultiplier, true);
      break;    
  }
}


boolean CheckForFirePowerIncrease(boolean powerComplete) {

  if (!powerComplete) {
    if (FireCompletions[CurrentPlayer]==PowerCompletions[CurrentPlayer]) {
      // The fire & power levels are equal, so we can advance
      PlaySoundEffect(SOUND_EFFECT_FIRE_INCREASED);
      FireLevelChangedTime = CurrentTime;
      return true;
    } else if (FireCompletions[CurrentPlayer]==(PowerCompletions[CurrentPlayer]-1)) {
      // The fire level is one behind the power, so we can advance AND increase the FirePowerLevel
      GiveFirePowerAward();
      FirePowerLevel[CurrentPlayer] += 1;
      PlaySoundEffect(SOUND_EFFECT_FIREPOWER_AWARD);
      FirePowerLevelChangedTime = CurrentTime;
      Audio.PlaySoundCardWhenPossible(31 * 256, CurrentTime+3500, 0, 500, 10);
      Audio.PlaySoundCardWhenPossible(19 * 256, CurrentTime+6000, 0, 500, 10);
      return true;
    }
  } else {
    if (FireCompletions[CurrentPlayer]==PowerCompletions[CurrentPlayer]) {
      // The fire & power levels are equal, so we can advance
      PlaySoundEffect(SOUND_EFFECT_POWER_INCREASED);
      PowerLevelChangedTime = CurrentTime;
      return true;
    } else if (PowerCompletions[CurrentPlayer]==(FireCompletions[CurrentPlayer]-1)) {
      // The fire level is one behind the power, so we can advance AND increase the FirePowerLevel
      GiveFirePowerAward();
      FirePowerLevel[CurrentPlayer] += 1;
      PlaySoundEffect(SOUND_EFFECT_FIREPOWER_AWARD);
      FirePowerLevelChangedTime = CurrentTime;
      Audio.PlaySoundCardWhenPossible(31 * 256, CurrentTime+3500, 0, 500, 10);
      Audio.PlaySoundCardWhenPossible(19 * 256, CurrentTime+6000, 0, 500, 10);
      return true;
    }
  }

  return false;
}


void HandleFireLane(byte switchNum) {
  if (switchNum<SW_TOP_F || switchNum>(SW_TOP_F+3)) return;
  byte fireID = (switchNum-SW_TOP_F);
  if ( FireLaneHitTime[fireID] && CurrentTime<(FireLaneHitTime[fireID]+250) ) return;
  FireLaneHitTime[fireID] = CurrentTime;
  
  SwitchHitsInMode += 1;
  byte fireBit = 0x01 << fireID;

  if (GameMode==GAME_MODE_BOSS_BATTLE) {
    if (BossBattleStage==BOSS_STAGE_FIRE_LANES) AddBossBattleHit();
    else PlaySoundEffect(SOUND_EFFECT_FIRE_HIT);
  } else {

    if (GameMode==GAME_MODE_SKILL_SHOT) {

      if (fireID==SkillShotLane) {
  //      QueueNotification(SOUND_EFFECT_VP_SKILL_SHOT_1 + CurrentTime%7, 5);
        PlaySoundEffect(SOUND_EFFECT_VP_SKILL_SHOT_1 + CurrentTime%7);
        SkillShotsHit[CurrentPlayer] += 1;

        unsigned long skillShotAward = SKILL_SHOT_REWARD * ((unsigned long)SkillShotsHit[CurrentPlayer]);
        if (!SkillShotChangedAfterLaunch) skillShotAward *= 2;
        Display_StartScoreAnimation(skillShotAward, true);
        if (SkillShotChangesWhenHit==true) {
          // We're going to change the skillshot because the player hit it
          SkillShotType[CurrentPlayer] = (SkillShotType[CurrentPlayer] + 1)%SKILL_SHOT_NUMBER_OF_TYPES;
        }

        // If they hit a skill shot, then we'll spot
        // them the next letter they need (if this is 
        // a duplicate switch)
        if (FireStatus[CurrentPlayer] & fireBit) {
          // This is a dupe, so change it
          for (byte count=0; count<4; count++) {
            fireID = count;
            fireBit = 0x01 << fireID;
            if ( (FireStatus[CurrentPlayer] & fireBit)==0 ) break;
          }
        }
      }
    }

    if (FireStatus[CurrentPlayer] & fireBit) {
      // this is a re-hit
      PlaySoundEffect(SOUND_EFFECT_FIRE_REHIT);
    } else {
      // this is a new hit
      FireStatus[CurrentPlayer] |= fireBit;
  
      if (DEBUG_MESSAGES) {
        Serial.write("Fire=");
        Serial.print(FireStatus[CurrentPlayer]);
        Serial.println();
      }
      if (FireStatus[CurrentPlayer]==0x0F) {
        // Bank is completed
        FireCompletedTime = CurrentTime;
        if (CheckForFirePowerIncrease(false)) {
          FireCompletions[CurrentPlayer] += 1;
        }
        FireStatus[CurrentPlayer] = 0;
        if (LockRequiredForShipUpgrades==false) {
          if (ShipWeapons[CurrentPlayer]<3) {
            ShipWeapons[CurrentPlayer] += 1;
            QueueNotification(SOUND_EFFECT_VP_WEAPONS_UPGRADED, 5);
          }
        } else {
          if (PlayerLocks[CurrentPlayer]) {
            if (ShipWeapons[CurrentPlayer]<3) {
              ShipWeapons[CurrentPlayer] += 1;
              QueueNotification(SOUND_EFFECT_VP_WEAPONS_UPGRADED, 5);
            }
          }
        }
        if (IncreaseBonusX()) {
          PlaySoundEffect(SOUND_EFFECT_FIRE_COMPLETED);
        } else {
          PlaySoundEffect(SOUND_EFFECT_FIRE_HIT);
        }
      } else {
        // Bank is not completed
        PlaySoundEffect(SOUND_EFFECT_FIRE_HIT);
      }
    }
  }
}


void HandlePowerStandup(byte switchNum) {

  byte powerID = (switchNum-SW_POWER_1);
  byte powerBit = 0x01 << powerID;

  if (PowerTargetHitTime[powerID] && CurrentTime<(PowerTargetHitTime[powerID]+250)) return;
  PowerTargetHitTime[powerID] = CurrentTime;
  SwitchHitsInMode += 1;

  if (GameMode==GAME_MODE_BOSS_BATTLE) {
    if (BossBattleStage==BOSS_STAGE_POWER_STANDUPS) AddBossBattleHit();
    else PlaySoundEffect(SOUND_EFFECT_POWER_HIT_1 + CurrentTime%2);
    return;
  }

  // Combo with the left inlane to power standups
  // can complete the power lamps
  if (ComboCompletesPower && (!PowerComboCollected) && LeftInlaneStage) {
    PowerCompletedTime = CurrentTime;
    PowerComboCollected = true;
    if (CheckForFirePowerIncrease(true)) {
      PowerCompletions[CurrentPlayer] += 1;
    }
    PowerStatus[CurrentPlayer] = 0;
    if (LockRequiredForShipUpgrades==false) {
      if (ShipThrusters[CurrentPlayer]<3) {
        ShipThrusters[CurrentPlayer] += 1;
        QueueNotification(SOUND_EFFECT_VP_THRUSTERS_UPGRADED, 5);
      }
    } else {
      if (PlayerLocks[CurrentPlayer]) {
        if (ShipThrusters[CurrentPlayer]<3) {
          ShipThrusters[CurrentPlayer] += 1;
          QueueNotification(SOUND_EFFECT_VP_THRUSTERS_UPGRADED, 5);
        }
      }
    }    
  } else if (PowerStatus[CurrentPlayer] & powerBit) {
    // this is a re-hit
    PlaySoundEffect(SOUND_EFFECT_POWER_REHIT);
  } else {
    // this is a new hit
    PowerStatus[CurrentPlayer] |= powerBit;
    if (PowerStatus[CurrentPlayer]==0x07) {
      // Bank is completed
      PowerCompletedTime = CurrentTime;
      if (CheckForFirePowerIncrease(true)) {
        PowerCompletions[CurrentPlayer] += 1;
      }
      PowerStatus[CurrentPlayer] = 0;
      if (LockRequiredForShipUpgrades==false) {
        if (ShipThrusters[CurrentPlayer]<3) {
          ShipThrusters[CurrentPlayer] += 1;
          QueueNotification(SOUND_EFFECT_VP_THRUSTERS_UPGRADED, 5);
        }
      } else {
        if (PlayerLocks[CurrentPlayer]) {
          if (ShipThrusters[CurrentPlayer]<3) {
            ShipThrusters[CurrentPlayer] += 1;
            QueueNotification(SOUND_EFFECT_VP_THRUSTERS_UPGRADED, 5);
          }
        }
      }
    } else {
      // Bank is not completed
      PlaySoundEffect(SOUND_EFFECT_POWER_HIT_1 + CurrentTime%2);
    }
  }  
}


void HandleRightFlipperSwitch() {
  if (GameMode==GAME_MODE_SKILL_SHOT) {
    if (SkillShotType[CurrentPlayer]==SKILL_SHOT_TYPE_ALWAYS_CHANGEABLE_LANE) {
      SkillShotLane = (SkillShotLane + 1)%4;
      if (!RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) SkillShotChangedAfterLaunch = true;
    } else if (SkillShotType[CurrentPlayer]==SKILL_SHOT_TYPE_SHOOTER_CHANGEABLE_LANE) {
      if (RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) SkillShotLane = (SkillShotLane + 1)%4;
    } else if (SkillShotType[CurrentPlayer]==SKILL_SHOT_TYPE_AUTO_CHANGING_LANE) {
      // This will be updated in ManageGameMode by the timer that changes lanes
    }

  } else if (GameMode==GAME_MODE_UNSTRUCTURED_PLAY || GameMode==GAME_MODE_BATTLE_1 || GameMode==GAME_MODE_BATTLE_2 || GameMode==GAME_MODE_BATTLE_3) {
    byte newFireStatus = FireStatus[CurrentPlayer];
    newFireStatus = newFireStatus * 2;
    if (newFireStatus & 0x10) {
      newFireStatus |= 0x01;
      newFireStatus &= 0x0F;
    }
    FireStatus[CurrentPlayer] = newFireStatus;

    unsigned long tempHit = FireLaneHitTime[3];
    FireLaneHitTime[3] = FireLaneHitTime[2];
    FireLaneHitTime[2] = FireLaneHitTime[1];
    FireLaneHitTime[1] = FireLaneHitTime[0];
    FireLaneHitTime[0] = tempHit;
  }
}

void HandlePopBumperHit(byte switchNum) {
  // POP IDs
  // 0 - bottom left
  // 1 - top left
  // 2 - top right
  // 3 - bottom right
  byte popID = switchNum - SW_BOTTOM_LEFT_POP;

  // Debounce pop scoring to every 200ms
  if (CurrentTime<(LastTimePopHit[popID]+200)) return;
  LastTimePopHit[popID] = CurrentTime;
  SwitchHitsInMode += 1;

  if (GameMode==GAME_MODE_BOSS_BATTLE && BossBattleStage==BOSS_STAGE_POP_BUMPERS) {
    AddBossBattleHit();
  }
  
  unsigned long popValue = 100;
  if (WeaponsTrainingHits[CurrentPlayer]) {
    popValue += ((unsigned long)WeaponsTrainingHits[CurrentPlayer] * 20);
  }
  CurrentScores[CurrentPlayer] += popValue * PlayfieldMultiplier;
  PlaySoundEffect(SOUND_EFFECT_POP_BUMPER);

  if (PlayerTrainingStatus[CurrentPlayer] & WEAPONS_TRAINING_RUNNING) {
    LastTimeWeaponsTrainingHit = CurrentTime;
    LastTrainingHitTime = CurrentTime;
    // We're in weapons training, so we will award weapons training
    // points and track pops to the goal
    WeaponsTrainingHits[CurrentPlayer] += 1;
    if (WeaponsTrainingHits[CurrentPlayer]>=100) FlightTrainingHits[CurrentPlayer] = 99;    
    CurrentScores[CurrentPlayer] += PlayfieldMultiplier * ((unsigned long)1000);
  } else {
    if (  (PlayerTrainingStatus[CurrentPlayer] & WEAPONS_TRAINING_QUALIFIED)==0 && 
          GameMode==GAME_MODE_UNSTRUCTURED_PLAY && 
          PopHits[CurrentPlayer]<CalculateShotsNeededForNextTraining(WEAPONS_TRAINING)) {
      PopHits[CurrentPlayer] += 1;
      if ( (PopHits[CurrentPlayer]%5)==1 ) {
        LastTimeWeaponsTrainingHit = CurrentTime;
        LastTrainingHitTime = CurrentTime;
      }
      if (PopHits[CurrentPlayer]>=CalculateShotsNeededForNextTraining(WEAPONS_TRAINING)) {
        QualifyPlayerTraining(WEAPONS_TRAINING);
      }
    }
  }

}

unsigned long LastLeftRolloverHitTime = 0;
unsigned long LastRightRolloverHitTime = 0;
void HandleStarRollover(byte associatedSaucer) {

  // Debound switch hit to 100ms
  if (associatedSaucer==0) {
    if (LastLeftRolloverHitTime && CurrentTime<(LastLeftRolloverHitTime+100)) return;
    LastLeftRolloverHitTime = CurrentTime;
  } else {
    if (LastRightRolloverHitTime && CurrentTime<(LastRightRolloverHitTime+100)) return;
    LastRightRolloverHitTime = CurrentTime;
  }

  SwitchHitsInMode += 1;
  PlaySoundEffect(SOUND_EFFECT_REFUELING);
  CurrentScores[CurrentPlayer] += 1000 * PlayfieldMultiplier;
}


void Handle10PointSwitch(byte switchHit) {
/*  
  byte switchBit = 0;

  switch (switchHit) {
    case SW_UPPER_MID_LEFT_10PT:
      switchBit = 0x01;
      break;
    case SW_TOP_LEFT_10PT:
      switchBit = 0x02;
      break;
    case SW_UPPER_TOP_RIGHT_10PT:
      switchBit = 0x04;
      break;
    case SW_LOWER_TOP_RIGHT_10PT:
      switchBit = 0x08;
      break;
    case SW_MIDDLE_RIGHT_10PT:
      switchBit = 0x10;
      break;
    case SW_LOWER_RIGHT_10PT:
      switchBit = 0x20;
      break;
    case SW_CENTER_MIDDLE_LEFT_10PT:
      switchBit = 0x40;
      break;
    case SW_LOWER_MIDDLE_LEFT_10PT:
      switchBit = 0x80;
      break;
  }
*/

  // if we've already seen this switch, we're done
//  if (TenPointSwitchHits[CurrentPlayer]&switchBit) {
    SwitchHitsInMode += 1;
    CurrentScores[CurrentPlayer] += 10 * PlayfieldMultiplier;
    PlaySoundEffect(SOUND_EFFECT_10PT_REHIT);
    (void)switchHit;
    return;
//  }


}



void HandleGamePlaySwitches(byte switchHit) {

  switch (switchHit) {

    case SW_TOP_F:
    case SW_TOP_I:
    case SW_TOP_R:
    case SW_TOP_E:
      HandleFireLane(switchHit);
      ValidateAndRegisterPlayfieldSwitch();
      break;
      
    case SW_RIGHT_FLIPPER:
      HandleRightFlipperSwitch();
      break;

    case SW_POWER_1:
    case SW_POWER_2:
    case SW_POWER_3:
      HandlePowerStandup(switchHit);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_UPPER_MID_LEFT_10PT:
    case SW_TOP_LEFT_10PT:
    case SW_UPPER_TOP_RIGHT_10PT:
    case SW_LOWER_TOP_RIGHT_10PT:
    case SW_MIDDLE_RIGHT_10PT:
    case SW_LOWER_RIGHT_10PT:
    case SW_CENTER_MIDDLE_LEFT_10PT:
    case SW_LOWER_MIDDLE_LEFT_10PT:
      Handle10PointSwitch(switchHit);
      break;

    case SW_1_STANDUP:
    case SW_2_STANDUP:
    case SW_3_STANDUP:
    case SW_4_STANDUP:
    case SW_5_STANDUP:
    case SW_6_STANDUP:
      HandleStandupHit(switchHit);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_TOP_LEFT_POP:
    case SW_TOP_RIGHT_POP:
    case SW_BOTTOM_LEFT_POP:
    case SW_BOTTOM_RIGHT_POP:
      HandlePopBumperHit(switchHit);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_LEFT_INLANE:
      if (CurrentTime>(LeftInlaneLastHitTime+250)) {
        SwitchHitsInMode += 1;
        AddToBonus(LeftInlaneStage + 1);
        CurrentScores[CurrentPlayer] += (1000 * (unsigned long)LeftInlaneStage * PlayfieldMultiplier);
        LeftInlaneLastHitTime = CurrentTime;
        PowerComboCollected = false;
        if (LeftInlaneStage<4) {
          LeftInlaneStage += 1;
          if (RightInlaneLastHitTime) {
            if (RightInlaneStage<4) {
              RightInlaneLastHitTime = CurrentTime;
              RightInlaneStage += 1;
            }
          }
        }
        PlaySoundEffect(SOUND_EFFECT_INLANE_1);
      }
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_RIGHT_INLANE:
      if (CurrentTime>(RightInlaneLastHitTime+250)) {
        SwitchHitsInMode += 1;
        AddToBonus(RightInlaneStage + 1);
        CurrentScores[CurrentPlayer] += (1000 * (unsigned long)RightInlaneStage * PlayfieldMultiplier);
        RightInlaneLastHitTime = CurrentTime;
        if (RightInlaneStage<4) {
          RightInlaneStage += 1;
          if (LeftInlaneLastHitTime) {
            if (LeftInlaneStage<4) {
              LeftInlaneLastHitTime = CurrentTime;
              LeftInlaneStage += 1;
            }
          }
        }      
        PlaySoundEffect(SOUND_EFFECT_INLANE_1);
      }
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_LEFT_OUTLANE:
      SetBallSave(4000, 0, true);
      if (LeftKickback[CurrentPlayer]) {
        PlaySoundEffect(SOUND_EFFECT_FIRE_HIT);
        RPU_PushToSolenoidStack(SOL_BALLSAVE_KICKER, 50, true);
        if (BallSaveEndTime==0) {
          if (BallKickerBehavior==BALL_KICKER_EASY) BallKickerEndTime = CurrentTime + 5000;
          else if (BallKickerBehavior==BALL_KICKER_MEDIUM) BallKickerEndTime = CurrentTime + 2000;
          else if (BallKickerBehavior==BALL_KICKER_HARD) BallKickerEndTime = CurrentTime + 1000;
          else LeftKickback[CurrentPlayer] -= 1;
        }
      } else {
        PlaySoundEffect(SOUND_EFFECT_OUTLANE_1);
      }
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_RIGHT_OUTLANE:
      PlaySoundEffect(SOUND_EFFECT_OUTLANE_1);
      SetBallSave(4000, 0, true);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_LEFT_SLINGSHOT:
      SwitchHitsInMode += 1;
      CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10;
      PlaySoundEffect(SOUND_EFFECT_LEFT_SLING);
      if (GameMode!=GAME_MODE_SKILL_SHOT) ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_RIGHT_SLINGSHOT:
      SwitchHitsInMode += 1;
      CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10;
      PlaySoundEffect(SOUND_EFFECT_RIGHT_SLING);
      if (GameMode!=GAME_MODE_SKILL_SHOT) ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_SPINNER:
      SwitchHitsInMode += 1;    
      HandleSpinnerProgress();
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_TOP_CENTER_BULLSEYE:
      HandleBullseye();
      ValidateAndRegisterPlayfieldSwitch();
      break;
    
    case SW_LEFT_SAUCER:
    case SW_RIGHT_SAUCER:
    case SW_TOP_SAUCER:
      // Saucer kitckout is now handled by debouncer
      // We'll validate the PF with this switch, unless
      // we just fired the Saucer as part of the ball search.
      //ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_LEFT_SAUCER_ROLLOVER:
      HandleStarRollover(0);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_RIGHT_SAUCER_ROLLOVER:
      HandleStarRollover(2);
      ValidateAndRegisterPlayfieldSwitch();
      break;

  }

}


int RunGamePlayMode(int curState, boolean curStateChanged) {
  int returnState = curState;
  unsigned long scoreAtTop = CurrentScores[CurrentPlayer];

  // Very first time into gameplay loop
  if (curState == MACHINE_STATE_INIT_GAMEPLAY) {
    returnState = InitGamePlay(curStateChanged);
  } else if (curState == MACHINE_STATE_INIT_NEW_BALL) {
    returnState = InitNewBall(curStateChanged);
  } else if (curState == MACHINE_STATE_NORMAL_GAMEPLAY) {
    returnState = ManageGameMode();
  } else if (curState == MACHINE_STATE_COUNTDOWN_BONUS) {
    Display_ClearOverride(0xFF);
    Display_UpdateDisplays(0xFF, true);
    returnState = CountdownBonus(curStateChanged);
//    ShowPlayerScoresOnTwoDisplays(0xFF, false, false);
  } else if (curState == MACHINE_STATE_BALL_OVER) {
    RPU_SetDisplayCredits(Credits, !FreePlayMode);

    if (SamePlayerShootsAgain) {
      QueueNotification(SOUND_EFFECT_VP_SHOOT_AGAIN, 10);
      returnState = MACHINE_STATE_INIT_NEW_BALL;
    } else {

      CurrentPlayer += 1;
      if (CurrentPlayer >= CurrentNumPlayers) {
        CurrentPlayer = 0;
        CurrentBallInPlay += 1;
      }

      scoreAtTop = CurrentScores[CurrentPlayer];

      if (CurrentBallInPlay > BallsPerGame) {
        EjectAllBallsFromSaucers();
        CheckHighScores();
        PlaySoundEffect(SOUND_EFFECT_GAME_OVER);
        for (int count = 0; count < CurrentNumPlayers; count++) {
          RPU_SetDisplay(count, CurrentScores[count], true, 2);
        }

        for (byte count = 0; count < 4; count++) {
          RPU_SetLampState(PlayerUpLamps[count], 0);
        }

        if (MatchEnabled) {
          returnState = MACHINE_STATE_MATCH_MODE;
        } else {
          returnState = MACHINE_STATE_ATTRACT;
        }
      }
      else returnState = MACHINE_STATE_INIT_NEW_BALL;
    }
  } else if (curState == MACHINE_STATE_MATCH_MODE) {
    returnState = ShowMatchSequence(curStateChanged);
  }

  byte switchHit;
  unsigned long lastBallFirstSwitchHitTime = BallFirstSwitchHitTime;

  while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
    returnState = HandleSystemSwitches(curState, switchHit);
    if (NumTiltWarnings <= MaxTiltWarnings) HandleGamePlaySwitches(switchHit);
  }

  if (CreditResetPressStarted) {
    if (CurrentBallInPlay < 2) {
      // If we haven't finished the first ball, we can add players
      AddPlayer();
      if (DEBUG_MESSAGES) {
        Serial.write("Start game button pressed\n\r");
      }
      CreditResetPressStarted = 0;
    } else {
      if (RPU_ReadSingleSwitchState(SW_CREDIT_RESET)) {
        if (TimeRequiredToResetGame != 99 && (CurrentTime - CreditResetPressStarted) >= ((unsigned long)TimeRequiredToResetGame * 1000)) {
          // If the first ball is over, pressing start again resets the game
          if (Credits >= 1 || FreePlayMode) {
            if (!FreePlayMode) {
              Credits -= 1;
              RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
              RPU_SetDisplayCredits(Credits, !FreePlayMode);
            }
            returnState = MACHINE_STATE_INIT_GAMEPLAY;
            CreditResetPressStarted = 0;
          }
        }
      } else {
        CreditResetPressStarted = 0;
      }
    }

  }

  if (lastBallFirstSwitchHitTime == 0 && BallFirstSwitchHitTime != 0) {
    BallSaveEndTime = BallFirstSwitchHitTime + ((unsigned long)BallSaveNumSeconds) * 1000;
    NumberOfBallSavesRemaining = 1;
  }
  if (CurrentTime > (BallSaveEndTime + BALL_SAVE_GRACE_PERIOD)) {
    BallSaveEndTime = 0;
    NumberOfBallSavesRemaining = 0;
  }

  if (!ScrollingScores && CurrentScores[CurrentPlayer] > RPU_OS_MAX_DISPLAY_SCORE) {
    CurrentScores[CurrentPlayer] -= RPU_OS_MAX_DISPLAY_SCORE;
    if (!TournamentScoring) AddSpecialCredit();
  }

  if (scoreAtTop != CurrentScores[CurrentPlayer]) {
    Display_SetLastTimeScoreChanged(CurrentTime);
    if (!TournamentScoring) {
      for (int awardCount = 0; awardCount < 3; awardCount++) {
        if (AwardScores[awardCount] != 0 && scoreAtTop < AwardScores[awardCount] && CurrentScores[CurrentPlayer] >= AwardScores[awardCount]) {
          // Player has just passed an award score, so we need to award it
          if (((ScoreAwardReplay >> awardCount) & 0x01)) {
            AddSpecialCredit();
          } else {
            AwardExtraBall(true);
          }
        }
      }
    }

  }

  return returnState;
}


#if (RPU_MPU_ARCHITECTURE>=10)
unsigned long LastLEDUpdateTime = 0;
byte LEDPhase = 0;
#endif

#ifdef DEBUG_SHOW_LOOPS_PER_SECOND
unsigned long NumLoops = 0;
unsigned long LastLoopReportTime = 0;
#endif

void loop() {

  CurrentTime = millis();
  int newMachineState = MachineState;
  
#ifdef DEBUG_SHOW_LOOPS_PER_SECOND
  NumLoops += 1;
  if (LastLoopReportTime==0) LastLoopReportTime = CurrentTime;
  if (CurrentTime>(LastLoopReportTime+1000)) {
    LastLoopReportTime = CurrentTime;
    char buf[128];
    sprintf(buf, "Loop running at %lu Hz (MachineState=%d, menu=%d)\n", NumLoops, MachineState, Menus.OperatorMenusActive());
    Serial.write(buf);
    NumLoops = 0;
  }
#endif

  if (Menus.OperatorMenusActive()) {
    RunOperatorMenu();
  } else {
    FirstTimeThroughOperatorMenu = true;
    if (RestoreBackgroundTrack!=BACKGROUND_TRACK_NONE) {
      PlayBackgroundSong(RestoreBackgroundTrack);
      RestoreBackgroundTrack = BACKGROUND_TRACK_NONE;
    }
    
    if (MachineState < 0) {
      newMachineState = 0;
    } else if (MachineState == MACHINE_STATE_ATTRACT) {
      newMachineState = RunAttractMode(MachineState, MachineStateChanged);
    } else if (MachineState == MACHINE_STATE_DIAGNOSTICS) {
      newMachineState = RunDiagnosticsMode(MachineState, MachineStateChanged);
    } else {
      newMachineState = RunGamePlayMode(MachineState, MachineStateChanged);
    }
  
    if (newMachineState != MachineState) {
      MachineState = newMachineState;
      MachineStateChanged = true;
    } else {
      MachineStateChanged = false;
    }
  }
  
  RPU_Update(CurrentTime);
  Audio.Update(CurrentTime);

#if (RPU_MPU_ARCHITECTURE>=10)
  if (LastLEDUpdateTime == 0 || (CurrentTime - LastLEDUpdateTime) > 250) {
    LastLEDUpdateTime = CurrentTime;
    RPU_SetBoardLEDs((LEDPhase % 8) == 1 || (LEDPhase % 8) == 3, (LEDPhase % 8) == 5 || (LEDPhase % 8) == 7);
    LEDPhase += 1;
  }
#endif

}
