/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>

#include "ASSETS.H"
#include "AUDIO.H"
#include "COMMON.H"
#include "GAME.H"
#include "MOUSE.H"
#include "SCRATCH.H"
#include "UTIL.H"
#include "VIDEO.H"

VIEW *current_view;
uchar quitting;

VIEW *view_stack[3];
uchar view_stack_i;

static void init_scene(VIEW *scene);

static VIEW *init_vault() {
  VIEW *vault = create_image_view("VAULT\\VAULT.RMG");
  VIEW *temp, *temp2;

  // Banker
  temp = add_inventory(
      vault, 444, 84, "VAULT\\BANKER_I.RMG", "VAULT\\BANKER.RMG",
      "Surely it's that welder's boy, " BOY_FIRST "!", BANKER_FIRST);
  add_clue(temp, BOY_FIRST, NULL, Person);
  temp2 = add_text_item(temp, 0,
                        ""
                        "Thank you for a wonderful evening.\n"
                        "I'm so glad I was able to change your\n"
                        "mind. You won't regret it, I promise.\n"
                        "Apologies for the necklace disaster,\n"
                        "I'm very clumsy with these things.\n"
                        "I will buy you a dozen new ones, My\n"
                        "Sweet Thing.\n\n" BANKER_FIRST,
                        STYLE_JAGGED);
  add_clue(temp2, BANKER_FIRST, NULL, Person);
  add_text_item(
      temp, 1,
      ""
      "He's coming out today, sir.\n"
      "  How many years has it been? 4? 5? Feels too soon.\n"
      "  We'd hate to see him go back to his old ways, wouldn't we.\n"
      "I will make the arrangements.\n"
      "  Don't go overboard, it might get expensive.\n"
      "He took it, sir.",
      STYLE_JAGGED);
  add_text_item(temp, 2, "Pack of cigarettes.", STYLE_DEFAULT);

  // Assistant
  temp = add_inventory(vault, 242, 84, "VAULT\\ASSIST_I.RMG",
                       "VAULT\\DETECTAS.RMG",
                       "Why didn't they cut all of it open?", " Policeman ");
  add_clue(temp, "open", "opened", Verb);
  temp2 = add_text_item(
      temp, 0,
      "ZIEGLER ALARM SYSTEM MANUAL\n"
      "---------------------------\n"
      "STRICTLY CONFIDENTIAL\n\n"
      "...\n"
      "To disable the alarm, the customer should call our operations\n"
      "center over the secure phone line and provide their registered\n"
      "password. In exceptional cases, the alarm can also be disabled\n"
      "by a Ziegler AG security agent using a secret passkey.\n"
      "...",
      STYLE_PRINT);
  add_clue(temp2, "password", NULL, Noun);
  add_clue(temp2, "passkey", NULL, Noun);
  temp = create_image_view("VAULT\\ALARM.RMG");
  temp2 = add_view(vault, 260, 56, temp);
  add_clue(add_text(temp2, 155, 47, "An empty slot inside the alarm.",
                    STYLE_DEFAULT),
           "alarm", NULL, Noun);

  // Hole
  temp = add_text(vault, 320, 93,
                  "Hole cut in the side\nof the safe with a torch.",
                  STYLE_DEFAULT);
  add_clue(temp, "cut", NULL, Verb);
  add_clue(temp, "safe", NULL, Noun);

  // Detective
  temp = add_inventory(
      vault, 112, 143, "VAULT\\DETECT_I.RMG", "VAULT\\DETECTIV.RMG",
      "I don't expect you'll give us the\nculprit this time, too, will you "
      "now?",
      " Detective ");
  add_clue(temp, "give", NULL, Verb);
  temp2 = create_text_view("FINGERPRINTS COLLECTED\n\nMr. " BOY_LAST
                           "      \nMr. " ROBBER_LAST "\nMs. " GIRL_LAST
                           "\nMr. " BANKER_LAST "\nMr. " ASSISTANT_LAST,
                           STYLE_PRINT);
  add_view(temp2, 112, 9 + 19, create_fingerprint_view("VAULT\\PRINT4.RMG"));
  add_view(temp2, 112, 18 + 19, create_fingerprint_view("VAULT\\PRINT2.RMG"));
  add_view(temp2, 112, 27 + 19, create_fingerprint_view("VAULT\\PRINT3.RMG"));
  add_view(temp2, 112, 36 + 19, create_fingerprint_view("VAULT\\PRINT5.RMG"));
  add_view(temp2, 112, 45 + 19, create_fingerprint_view("VAULT\\PRINT1.RMG"));
  add_view_item(temp, 0, temp2);
  add_text_item(temp, 1, "Pack of cigarettes.", STYLE_DEFAULT);

  // Safe
  temp = create_image_view("VAULT\\SAFE.RMG");
  temp2 = add_text(temp, 186, 82, "2,628 Shillings", STYLE_DEFAULT);
  add_view(vault, 386, 76, temp);

  #include "VAULT.INC"

  return vault;
}

static VIEW *init_lobby() {
  VIEW *lobby = create_image_view("LOBBY\\LOBBY.RMG");
  VIEW *temp, *temp2;

  // Robber
  temp = add_inventory(lobby, 388, 65, "LOBBY\\ROBBER_I.RMG",
                       "LOBBY\\ROBBER.RMG", "Cheap fraud.", ROBBER_FIRST);
  add_clue(
      add_text_item(temp, 0,
                    "An empty money envelope with " BANKER_LAST " branding.",
                    STYLE_DEFAULT),
      "money", NULL, Noun);
  add_text_item(temp, 1,
                "" BANKER_LAST
                " Banking House\n"
                "INVESTMENT ACCOUNT STATEMENT\nSeptember 1934\n\n"
                "Account:          Mr. & Mrs. " BOY_LAST
                "\n\n"
                "Opening Balance:    2,000 S   0 Gr\n"
                "Profit/Loss:      +   100 S   0 Gr\n"
                "Fees:             - 1,234 S  56 Gr\n"
                "Closing Balance:      865 S  44 Gr",
                STYLE_PRINT);

  // Girl
  temp = add_inventory(lobby, 227, 50, "LOBBY\\GIRL_I.RMG", "LOBBY\\GIRL.RMG",
                       "That's absurd! Do you know who I am?!", GIRL_FIRST);
  temp2 = add_text_item(
      temp, 0,
      ""
      "Dear " GIRL_FIRST
      ",\n\n"
      "After the stunt you pulled at the factory today\n"
      "I can no longer tolerate the damage you have\n"
      "been inflicting on our family business through\n"
      "your wicked lifestyle and people you associate with.\n"
      "Your access has been revoked and you will no longer\n"
      "be included in my will.\n\n"
      "I wish you all the best with the choices you've made.\n\n"
      "Your Father, Heinrich August " GIRL_LAST,
      STYLE_JAGGED);
  add_text_item(temp, 1, "Pack of cigarettes.", STYLE_DEFAULT);
  add_clue(temp2, "Heinrich", NULL, Person);
  add_clue(temp2, GIRL_FIRST, NULL, Person);

  // Boyfriend
  temp = add_inventory(lobby, 532, 53, "LOBBY\\BOY_I.RMG", "LOBBY\\BOY.RMG",
                       "He's just jealous we're lovers.", BOY_FIRST);
  add_clue(temp, "lovers", NULL, Noun);
  temp2 = create_image_view("LOBBY\\NECKLACE.RMG");
  add_clue(add_text(temp2, 146, 25, "The necklace is missing a clasp.",
                    STYLE_DEFAULT),
           "necklace", NULL, Noun);
  add_view_item(temp, 0, temp2);
  temp2 = add_text_item(
      temp, 1,
      "Old fool! I stole it right from under his nose.\nIt fits perfectly. "
      "Today's the date.\nBe by my window at 8pm and I will drop it.\n\nLove "
      "XXX",
      STYLE_JAGGED);
  add_clue(temp2, "date", NULL, Noun);
  add_text_item(
      temp, 2,
      "Leave this with me.\n\nIf you do as I say, you can get it all "
      "back.",
      STYLE_JAGGED);
  add_text_item(temp, 3, "Pack of cigarettes.", STYLE_DEFAULT);

  // Policeman
  temp = add_inventory(
      lobby, 24, 90, "LOBBY\\POLICE_I.RMG", "LOBBY\\GUARD.RMG",
      "We will be collecting your statements\nshortly.", " Policeman ");
  temp2 = add_text_item(temp, 0,
                        "INTERVIEW LIST\n\n"
                        "Ms. " GIRL_LAST
                        "\n"
                        "  Mr. " BANKER_LAST
                        "'s alibi\n\n"
                        "Mr. " ASSISTANT_LAST
                        "\n"
                        "  Assistant to Mr. " BANKER_LAST
                        "\n\n"
                        "Mr. " BOY_LAST
                        "\n"
                        "  Indicated as potential suspect by Mr. " BANKER_LAST
                        "\n\n"
                        "Mr. " ROBBER_LAST
                        "\n"
                        "  Claims to have information about stolen money",
                        STYLE_HAND);
  add_clue(temp2, "stolen", NULL, Verb);

  temp = add_text(lobby, 148, 130,
                  "VAULT JOURNAL\n"
                  "7.12.1934 17:00 COB\n\n"
                  "Shelf               Amount\n"
                  "--------------------------------\n"
                  "  Top               0 S   0 Gr\n"
                  "  Middle       48,560 S  20 Gr\n"
                  "  Bottom       21,939 S  11 Gr\n\n"
                  "Counted by: " ASSISTANT_FIRST " " ASSISTANT_LAST,
                  STYLE_PRINT);
  add_clue(temp, ASSISTANT_FIRST, NULL, Person);

  temp = create_text_view(
      "Convicted Bank Robber\n\nCaught after having been betrayed by\nhis "
      "partner and recently released.",
      STYLE_HAND);
  add_clue(temp, "partner", "partners", Noun);
  add_view(lobby, 580, 143, temp);

  temp = create_text_view(
      "Disgruntled Customer\n\nSwore revenge on our bank after his\nparents' "
      "investments underperformed.\nDo not serve.",
      STYLE_HAND);
  add_clue(temp, "revenge", NULL, Noun);
  add_view(lobby, 538, 135, temp);

  #include "LOBBY.INC"

  return lobby;
}

static VIEW *init_bathroom() {
  VIEW *bathroom = create_image_view("BATHROOM\\BATHROOM.RMG");
  VIEW *temp, *temp2, *temp3;

  temp = add_inventory(bathroom, 474, 60, "BATHROOM\\BASS_I.RMG",
                       "BATHROOM\\BASS.RMG", "You've got to be kidding me.",
                       ASSISTANT_FIRST);
  temp2 = create_image_view("BATHROOM\\NEWSPAPR.RMG");
  add_text(
      temp2, 70, 81,
      "MINIMUM WAGE LAWS\n"
      "-----------------\n"
      "  The government has introduced new minimum wage laws for general "
      "labor\n"
      "with the rate set at 5 groschen per hour. Progressive parties have\n"
      "praised the new law as an exciting step forward in ensuring "
      "wellbeing\n"
      "of all layers of our society.\n"
      "  An additional proposal to cap weekly working hours has met with "
      "steep\n"
      "opposition from industrial lobbies and failed to pass through the\n"
      "parliament.",
      STYLE_PRINT);
  temp3 = add_text(
      temp2, 280, 85,
      "SAFECRACKER BACK ON THE STREETS\n"
      "-------------------------------\n"
      "  Infamous bank robber " ROBBER_FIRST " " ROBBER_LAST
      " will be released from prison tomorrow\n"
      "after serving 6 years for attempted robbery of Commerce Bank in "
      "Linz.\n"
      "His capture was the result of a sting operation sparked by an "
      "anonymous\n"
      "tip. Mr. " ROBBER_LAST
      " rose to notoriety by allegedly being able to open\n"
      "safes without any tools and is believed to have been involved in "
      "many\n"
      "robberies. He has always maintained a stance of mistaken identity.",
      STYLE_PRINT);
  add_clue(temp3, "robbery", NULL, Noun);
  add_clue(temp3, ROBBER_FIRST, NULL, Person);
  add_clue(
      add_text(
          temp2, 240, 40,
          "BANKING SCAMS ON THE RISE\n"
          "-------------------------\n"
          "  The OeNB has issued a warning to the public to be wary of "
          "widespread\n"
          "fraud perpetrated by unregulated credit unions and banking "
          "houses.\n"
          "In many cases deposits and investments made into such "
          "institutions\n"
          "were either defrauded, charged exorbitant fees or used to carry "
          "out\n"
          "insurance fraud.\n"
          "  The government is urging members of the public to be vigilant\n"
          "and only deal with established and reputable banks until a new "
          "task\n"
          "force is set up to combat this modern day epidemic.",
          STYLE_PRINT),
      "fraud", NULL, Noun);
  add_view_item(temp, 0, temp2);

  add_text_item(temp, 1,
                ""
                "INSURANCE CERTIFICATE\n"
                "Kegel Insurance AG\n\n"
                "Insured:  " BANKER_LAST
                " Banking House\n"
                "Start:    19.11.1934\n"
                "Term:     1 year\n"
                "Coverage: Theft\n"
                "Amount:   50,000 S\n\n"
                "Signed: " BANKER_INITIAL " " BANKER_LAST
                "     Signed: R. Kegel",
                STYLE_PRINT);
  add_text_item(temp, 2,
                ""
                "  2190\n"
                "   120\n"
                "  438\n"
                " 219 \n"
                " 262800",
                STYLE_JAGGED);

  add_text_item(temp, 3, "Pack of cigarettes.", STYLE_DEFAULT);
  add_text(bathroom, 340, 70, "Hole in the wall.", STYLE_DEFAULT);

  // Police technician
  temp = add_inventory(bathroom, 218, 94, "BATHROOM\\PASS_I.RMG",
                       "BATHROOM\\POLICEAS.RMG",
                       "Someone must have left this behind?", " Policeman ");
  add_clue(temp, "left", NULL, Verb);
  add_clue(add_text_item(temp, 0, "Fingerprint kit.", STYLE_DEFAULT),
           "fingerprint", NULL, Noun);

  temp = create_image_view("BATHROOM\\PLATE.RMG");
  temp2 = add_view(bathroom, 109, 133, temp);

  temp = create_image_view("BATHROOM\\CIGARETT.RMG");
  add_view(bathroom, 320, 144, temp);

  #include "BATHROOM.INC"

  return bathroom;
}

void game_init() {
  VIEW *scene_vault = init_vault();
  VIEW *scene_bathroom = init_bathroom();
  VIEW *scene_lobby = init_lobby();

  // transitions
  add_transition(scene_vault, 618, 78, scene_bathroom);
  add_transition(scene_vault, 11, 78, scene_lobby);
  add_transition(scene_bathroom, 11, 78, scene_vault);
  add_transition(scene_lobby, 618, 78, scene_vault);

  // convert sprites and hotspots to absolute positions
  init_scene(scene_vault);
  init_scene(scene_bathroom);
  init_scene(scene_lobby);

  current_view = scene_vault;
}

void init_scene(VIEW *scene) {
  VIEW *t = scene->targets;
  SPRITE *s = scene->sprites;
  CLUE *c = scene->popup.clues;

  if (scene->id == VIEW_TRANSITION)
    return;

  if (scene->input) {
    scene->input->x += scene->popup.x;
    scene->input->y += scene->popup.y;
  }
  while (c) {
    c->x += scene->popup.x;
    c->y += scene->popup.y;
    if (scene->id == VIEW_INVENTORY) {
      c->x += INV_MARGIN;
    }
    c = c->next;
  }
  while (s) {
    s->sx += scene->popup.x;
    s->sy += scene->popup.y;
    s = s->next;
  }
  while (t) {
    t->hs_x += scene->popup.x;
    t->hs_y += scene->popup.y;
    init_scene(t);
    t = t->next;
  }
}

void game_switch_scene(VIEW *scene) {
  if (view_stack_i)
    fatal_error("Switching scene with stack");
  audio_sfx(SFX_MOVE);
  video_fade_out();
  load_scene(scene);
  video_fade_in();
  current_view = scene;
}

void game_push_view(VIEW *view) {
  view_stack[view_stack_i++] = current_view;
  current_view = view;
}

int game_pop_view() {
  if (view_stack_i > 0) {
    hotspot_pop(current_view->num_targets);
    current_view = view_stack[--view_stack_i];
    return 1;
  }
  return 0;
}

int mouse_over_quiz() {
  return mouse_x >= 640 - QUIZ_BUTTON_W && mouse_y >= 200 - QUIZ_BUTTON_H;
}
