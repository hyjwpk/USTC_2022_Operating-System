
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unistd.h>
//#include "mm.h"
//#include "memlib.h"
// #include "hamlet.h"
//#include "config.h"
#include "zipf.hpp"

#define MAX_ITEMS 1000000
#define LOOP_NUM 5
#define SEED 10000
#define WORKLOAD_TYPE 16
#define malloc malloc
//#define free mm_free
unsigned int workload_size[WORKLOAD_TYPE] = {12, 16, 24, 32, 48, 64, 96, 100, 128, 192, 256, 384, 500, 512, 768, 1024};

extern size_t user_malloc_size;
extern size_t heap_size;
char *trace_data = "The early bird catches the worm. The early bird catches the worm. The early bird catches the worm.";
static char global_data[100] = "Stupid bird fly first. Stupid bird fly first. Stupid bird fly first.";
char hamlet_scene1[8276] = "ACT 1=====Scene 1=======[Enter Barnardo and Francisco, two sentinels.]BARNARDO  Who's there?FRANCISCONay, answer me. Stand and unfold yourself.BARNARDO  Long live the King!FRANCISCO  Barnardo?BARNARDO  He.FRANCISCOYou come most carefully upon your hour.BARNARDO'Tis now struck twelve. Get thee to bed, Francisco.FRANCISCOFor this relief much thanks. 'Tis bitter cold,And I am sick at heart.BARNARDO  Have you had quiet guard?FRANCISCO  Not a mouse stirring.BARNARDO  Well, good night.If you do meet Horatio and Marcellus,The rivals of my watch, bid them make haste.[Enter Horatio and Marcellus.]FRANCISCOI think I hear them.--Stand ho! Who is there?HORATIO  Friends to this ground.MARCELLUS  And liegemen to the Dane.FRANCISCO  Give you good night.MARCELLUSO farewell, honest soldier. Who hath relievedyou?FRANCISCOBarnardo hath my place. Give you good night.[Francisco exits.]MARCELLUS  Holla, Barnardo.BARNARDO  Say, what, is Horatio there?HORATIO  A piece of him.BARNARDOWelcome, Horatio.--Welcome, good Marcellus.HORATIOWhat, has this thing appeared again tonight?BARNARDO  I have seen nothing.MARCELLUSHoratio says 'tis but our fantasyAnd will not let belief take hold of himTouching this dreaded sight twice seen of us.Therefore I have entreated him alongWith us to watch the minutes of this night,That, if again this apparition come,He may approve our eyes and speak to it.HORATIOTush, tush, 'twill not appear.BARNARDO  Sit down awhile,And let us once again assail your ears,That are so fortified against our story,What we have two nights seen.HORATIO  Well, sit we down,And let us hear Barnardo speak of this.BARNARDO  Last night of all,When yond same star that's westward from the poleHad made his course t' illume that part of heavenWhere now it burns, Marcellus and myself,The bell then beating one--[Enter Ghost.]MARCELLUSPeace, break thee off! Look where it comes again.BARNARDOIn the same figure like the King that's dead.MARCELLUS, [to Horatio]Thou art a scholar. Speak to it, Horatio.BARNARDOLooks he not like the King? Mark it, Horatio.HORATIOMost like. It harrows me with fear and wonder.BARNARDOIt would be spoke to.MARCELLUS  Speak to it, Horatio.HORATIOWhat art thou that usurp'st this time of night,Together with that fair and warlike formIn which the majesty of buried DenmarkDid sometimes march? By heaven, I charge thee,speak.MARCELLUSIt is offended.BARNARDO  See, it stalks away.HORATIOStay! speak! speak! I charge thee, speak![Ghost exits.]MARCELLUS  'Tis gone and will not answer.BARNARDOHow now, Horatio, you tremble and look pale.Is not this something more than fantasy?What think you on 't?HORATIOBefore my God, I might not this believeWithout the sensible and true avouchOf mine own eyes.MARCELLUS  Is it not like the King?HORATIO  As thou art to thyself.Such was the very armor he had onWhen he the ambitious Norway combated.So frowned he once when, in an angry parle,He smote the sledded Polacks on the ice.'Tis strange.MARCELLUSThus twice before, and jump at this dead hour,With martial stalk hath he gone by our watch.HORATIOIn what particular thought to work I know not,But in the gross and scope of mine opinionThis bodes some strange eruption to our state.MARCELLUSGood now, sit down, and tell me, he that knows,Why this same strict and most observant watchSo nightly toils the subject of the land,And why such daily cast of brazen cannonAnd foreign mart for implements of war,Why such impress of shipwrights, whose sore taskDoes not divide the Sunday from the week.What might be toward that this sweaty hasteDoth make the night joint laborer with the day?Who is 't that can inform me?HORATIO  That can I.At least the whisper goes so: our last king,Whose image even but now appeared to us,Was, as you know, by Fortinbras of Norway,Thereto pricked on by a most emulate pride,Dared to the combat; in which our valiant Hamlet(For so this side of our known world esteemed him)Did slay this Fortinbras, who by a sealed compact,Well ratified by law and heraldry,Did forfeit, with his life, all those his landsWhich he stood seized of, to the conqueror.Against the which a moiety competentWas gaged by our king, which had returnedTo the inheritance of FortinbrasHad he been vanquisher, as, by the same comartAnd carriage of the article designed,His fell to Hamlet. Now, sir, young Fortinbras,Of unimproved mettle hot and full,Hath in the skirts of Norway here and thereSharked up a list of lawless resolutesFor food and diet to some enterpriseThat hath a stomach in 't; which is no other(As it doth well appear unto our state)But to recover of us, by strong handAnd terms compulsatory, those foresaid landsSo by his father lost. And this, I take it,Is the main motive of our preparations,The source of this our watch, and the chief headOf this posthaste and rummage in the land.BARNARDOI think it be no other but e'en so.Well may it sort that this portentous figureComes armed through our watch so like the kingThat was and is the question of these wars.HORATIOA mote it is to trouble the mind's eye.In the most high and palmy state of Rome,A little ere the mightiest Julius fell,The graves stood tenantless, and the sheeted deadDid squeak and gibber in the Roman streets;As stars with trains of fire and dews of blood,Disasters in the sun; and the moist star,Upon whose influence Neptune's empire stands,Was sick almost to doomsday with eclipse.And even the like precurse of feared events,As harbingers preceding still the fatesAnd prologue to the omen coming on,Have heaven and Earth together demonstratedUnto our climatures and countrymen.[Enter Ghost.]But soft, behold! Lo, where it comes again!I'll cross it though it blast me.--Stay, illusion![It spreads his arms.]If thou hast any sound or use of voice,Speak to me.If there be any good thing to be doneThat may to thee do ease and grace to me,Speak to me.If thou art privy to thy country's fate,Which happily foreknowing may avoid,O, speak!Or if thou hast uphoarded in thy lifeExtorted treasure in the womb of earth,For which, they say, you spirits oft walk in death,Speak of it.	[The cock crows.]Stay and speak!--Stop it, Marcellus.MARCELLUSShall I strike it with my partisan?HORATIO  Do, if it will not stand.BARNARDO  'Tis here.HORATIO  'Tis here.[Ghost exits.]MARCELLUS  'Tis gone.We do it wrong, being so majestical,To offer it the show of violence,For it is as the air, invulnerable,And our vain blows malicious mockery.BARNARDOIt was about to speak when the cock crew.HORATIOAnd then it started like a guilty thingUpon a fearful summons. I have heardThe cock, that is the trumpet to the morn,Doth with his lofty and shrill-sounding throatAwake the god of day, and at his warning,Whether in sea or fire, in earth or air,Th' extravagant and erring spirit hiesTo his confine, and of the truth hereinThis present object made probation.MARCELLUSIt faded on the crowing of the cock.Some say that ever 'gainst that season comesWherein our Savior's birth is celebrated,This bird of dawning singeth all night long;And then, they say, no spirit dare stir abroad,The nights are wholesome; then no planets strike,No fairy takes, nor witch hath power to charm,So hallowed and so gracious is that time.HORATIOSo have I heard and do in part believe it.But look, the morn in russet mantle cladWalks o'er the dew of yon high eastward hill.Break we our watch up, and by my adviceLet us impart what we have seen tonightUnto young Hamlet; for, upon my life,This spirit, dumb to us, will speak to him.Do you consent we shall acquaint him with itAs needful in our loves, fitting our duty?MARCELLUSLet's do 't, I pray, and I this morning knowWhere we shall find him most convenient.[They exit.]";
/*A simplified workload storage index*/
struct workload_base
{
    void **addr;
};

/*Generation of string with length*/
char *gen_random_string(int length)
{
    int flag, i;
    char *string;
    if ((string = (char *)malloc(length)) == NULL)
    {
        std::cerr << "Malloc failed at genRandomString!" << std::endl;
        return NULL;
    }

    for (i = 0; i < length - 1; i++)
    {
        flag = rand() % 3;
        switch (flag)
        {
        case 0:
            string[i] = 'A' + rand() % 26;
            break;
        case 1:
            string[i] = 'a' + rand() % 26;
            break;
        case 2:
            string[i] = '0' + rand() % 10;
            break;
        default:
            string[i] = 'x';
            break;
        }
    }
    string[length - 1] = '\0';
    return string;
}

/* Create the workload index */
int workload_create(struct workload_base *workload)
{
    srand(SEED);
    // mem_init();
    /*if (mm_init() < 0)
    {
        fprintf(stderr, "mm_init failed.\n");
        return 0;
    }*/
    workload->addr = (void **)malloc(sizeof(void *) * MAX_ITEMS);
    memset(workload->addr, 0, sizeof(void *) * MAX_ITEMS);
    return 0;
}

/* Insert strings up to 100% of MAX_ITEMS */
int workload_insert(struct workload_base *workload)
{
    unsigned int size, total = 0;
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        if (workload->addr[i] == 0)
        {
            size = workload_size[rand() % WORKLOAD_TYPE];
            workload->addr[i] = gen_random_string(size);
            total += size;
        }
    }
    return 0;
}

/* Sort strings */
int workload_swap(struct workload_base *workload)
{
    for (int i = 1; i < MAX_ITEMS; i++)
    {
        void *temp;
        temp = workload->addr[i];
        workload->addr[i] = workload->addr[i - 1];
        workload->addr[i - 1] = temp;
    }
    return 0;
}

/* Read strings, at a zipfian distribution */
int workload_read(struct workload_base *workload)
{
    char reader[1025];
    zipf_distribution<int, double> zipf(MAX_ITEMS - 1, 0.99);
    std::mt19937 generator2(SEED);
    for (int j = 0; j < 10; j++)
    {
        for (int i = 0; i < 100; i++)
        {
            strcpy(reader, (char *)workload->addr[zipf(generator2)]);
        }
        sleep(1);
    }
}

/* Randomly delete 80% of strings */
int workload_delete(struct workload_base *workload)
{
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        if (rand() % 5 != 0)
        {
            free(workload->addr[i]);
            workload->addr[i] = 0;
        }
    }
    return 0;
}

/* Run workload */
void *workload_run(void *workload)
{
    struct timeval cur_time;
    for (int loop = 0; loop < LOOP_NUM; loop++)
    {
        gettimeofday(&cur_time, NULL);
        long sec1 = cur_time.tv_sec, usec1 = cur_time.tv_usec;
        workload_insert((struct workload_base *)workload);
        workload_swap((struct workload_base *)workload);
        workload_read((struct workload_base *)workload);
        // std::cout<<"before free: "<<get_utilization();
        workload_delete((struct workload_base *)workload);
        // std::cout<<"; after free: "<<get_utilization()<<std::endl;
        gettimeofday(&cur_time, NULL);
        long sec2 = cur_time.tv_sec, usec2 = cur_time.tv_usec;
        std::cout << "time of loop " << loop << " : " << (sec2 - sec1) * 1000 + (usec2 - usec1) / 1000 << "ms" << std::endl;
    }
}

/* Run monitor */
/*void* monitor_run(void *argv){
    double util;
    std::ofstream fout;
    fout.open("./mem_util.csv", std::ios::out);
    long timer=0;
    fout<<"time\tutil"<<std::endl;
    while(1){
        util=get_utilization();
        fout<<timer<<"\t"<<util<<std::endl;
        timer++;
        sleep(1);
    }
    fout.close();
}*/

int main()
{
    int error;
    struct workload_base workload;
    if (error = workload_create(&workload))
    {
        std::cerr << "workload creat error:" << error << std::endl;
    }
    pthread_t monitor_pid;
    // pthread_create(&monitor_pid, NULL, monitor_run, NULL);
    workload_run(&workload);
    // pthread_cancel(monitor_pid);
    return 0;
}
