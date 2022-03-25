#include <stdio.h>
#include "brick.h"
#include <unistd.h>
//#include <graphics.h>
#define Sleep(msec) usleep((msec)*1000) /* Definerar sleep, Sleep(1000)= 1 sekund */

#define MOTOR_RIGHT     OUTA
#define MOTOR_MEDIUM    OUTB
#define MOTOR_C         OUTC
#define MOTOR_LEFT      OUTD
#define SENSOR_TOUCH    IN1
#define SENSOR_GYRO     IN2
#define SENSOR_COLOR    IN3
#define SENSOR_SONIC    IN4
#define MOTOR_BOTH      (MOTOR_LEFT|MOTOR_RIGHT) /* Bitvis ELLER ger att båda motorerna styrs samtidigt */

//Hastighet för olika motorer
int max_speed_big;         /* variabel för max hastighet på motorn */
int max_speed_medium;

//Sensoren
POOL_T sonicSensor;
POOL_T touchSensor;
POOL_T gyroSensor;
POOL_T colorSensor;


//Globala variabler
int TouchReturnValue = 0;
int gyroValue;
int superStartGyro;
float sonicValue;
int color;


//int gr = DETECT, gm;

//Funktioner
void unloadBook();
void rotate(int angle, char direction);
void find_wall_start(int distance);
void find_wall_angle(int distance);
void find_wall(int distance);
void go_forward(int distance, float speed);
void go_backwards(int distance, float speed);

int main(void){

    if(!brick_init()) return(1); /* Initialiserar EV3-klossen */
    printf("Woof! Hello, human! I am V.A.L.P., nice to see you!\n");
    Sleep(2000);

    if(tacho_is_plugged(MOTOR_BOTH|MOTOR_MEDIUM, TACHO_TYPE__NONE_)){
          /* TACHO_TYPE__NONE_ = Alla typer av motorer         */
        max_speed_big = tacho_get_max_speed(MOTOR_LEFT, 0);/* Kollar maxhastigheten som motorn kan ha         */
        max_speed_medium = tacho_get_max_speed(MOTOR_MEDIUM, 0);
        tacho_reset(MOTOR_BOTH);    
    } 
    else {        
        printf("Woof!\n Connect left motor in port D\n Connect right motor in port A\n Connect medium tacho motor in port B\n");
        brick_uninit();        
        
        return(0);  /* Stänger av sig om motorer ej är inkopplade     */
    }

    if(!sensor_is_plugged((SENSOR_TOUCH|SENSOR_GYRO|SENSOR_COLOR|SENSOR_SONIC), SENSOR_TYPE__NONE_)){
        printf("Woof!\n Connect touch sensor in port 1\n Connect gyro sensor in port 2\n Connect colour sensor in port 3 \n Connect ultrasonic sensor in port 4\n");
        brick_uninit();
        return(0);
    }

    touchSensor = sensor_search(LEGO_EV3_TOUCH);
    touch_set_mode_touch(touchSensor);
    gyroSensor = sensor_search(LEGO_EV3_GYRO);
    gyro_set_mode_gyro_g_and_a(gyroSensor);	
    sonicSensor = sensor_search(LEGO_EV3_US);
    us_set_mode_us_dist_cm(sonicSensor);
    colorSensor = sensor_search(LEGO_EV3_COLOR);
    color_set_mode_col_color(colorSensor);

    while(!TouchReturnValue){
        tacho_set_speed_sp(MOTOR_MEDIUM, max_speed_medium);
        tacho_run_forever(MOTOR_MEDIUM);
        TouchReturnValue = sensor_get_value(0, touchSensor, 0);
        color = sensor_get_value(0, colorSensor, 0);
        switch(color){
            case 6:
                //Uppdrag 1 Vit
                superStartGyro = sensor_get_value(0, gyroSensor, 0);
                find_wall_start(515);
                Sleep(1000);
                go_backwards(12, 0.3);
                Sleep(1000);
                rotate(90, 'r');
                Sleep(1000);
                go_forward(275, 0.4);
                unloadBook();
                break;
            case 3:
                //Uppdrag 2 Grön
                find_wall_start(515);
                Sleep(1000);
                go_backwards(12, 0.3);
                Sleep(1000);
                rotate(90, 'l');
                Sleep(1000);
                go_forward(285, 0.4);
                unloadBook();
                break;
            case 4:
                //Uppdrag 3 Gul
                find_wall_start(515);
                Sleep(1000);
                go_backwards(30, 0.3);
                Sleep(1000);
                rotate(90, 'r');
                Sleep(1000);
                go_forward(275, 0.4);
                Sleep(1000);
                rotate(90, 'r');
                find_wall(220);
                rotate(90, 'l');
                unloadBook();
                break;
            case 5:
                //Uppdrag 4 Röd
                find_wall_start(515);
                Sleep(1000);
                go_backwards(30, 0.3);
                Sleep(1000);
                rotate(90, 'l');
                Sleep(1000);
                go_forward(285, 0.4);
                Sleep(1000);
                rotate(90, 'l');
                find_wall(220);
                rotate(90, 'r');
                unloadBook();
                break;
        }
    }

    brick_uninit();
    printf("Woof... sleepy... uwu\n");    
    return (0);
}

void unloadBook(){

    tacho_stop(MOTOR_MEDIUM);
    tacho_set_speed_sp(MOTOR_MEDIUM, max_speed_medium*-1);
    tacho_run_forever(MOTOR_MEDIUM);
    Sleep(7000);
    tacho_stop(MOTOR_MEDIUM);
    printf("Woof! Book has been planted\n");
    
}

void rotate(int angle, char direction){

    gyroValue = sensor_get_value(0, gyroSensor, 0);
    int goal_angle = 0;
    switch(direction){
        case 'r':
            goal_angle = gyroValue+angle;
            tacho_set_speed_sp(MOTOR_LEFT, max_speed_big*0.2);
            tacho_set_speed_sp(MOTOR_RIGHT, max_speed_big*-0.2);
            break;
        case 'l':
            goal_angle = gyroValue-angle;
            tacho_set_speed_sp(MOTOR_LEFT, max_speed_big*-0.2);
            tacho_set_speed_sp(MOTOR_RIGHT, max_speed_big*0.2);
            break;
        default:
            printf("Woof! Wrong direction value! I can turn only 'r'ight and 'l'eft!");
            break;
    }
    while((gyroValue < (goal_angle-2)) || (gyroValue > (goal_angle+4))){
        tacho_run_forever(MOTOR_BOTH);
        gyroValue = sensor_get_value(0, gyroSensor, 0);
    }
    tacho_stop(MOTOR_BOTH);

}

void find_wall_start(int distance){


    float minSonic = 2000;
    int minGyro;
    int startGyro = sensor_get_value(0, gyroSensor, 0);
    gyroValue = startGyro;
    tacho_set_speed_sp(MOTOR_LEFT, max_speed_big*0.4);
    tacho_set_speed_sp(MOTOR_RIGHT, max_speed_big*-0.4);

    while(gyroValue < (startGyro + 360)){
        sonicValue = sensor_get_value(0, sonicSensor, 0);
        gyroValue = sensor_get_value(0, gyroSensor, 0);
        tacho_run_forever(MOTOR_BOTH);
        if(sonicValue < minSonic && sonicValue > 100){
            minSonic = sonicValue;
            minGyro = sensor_get_value(0, gyroSensor, 0);
        }
    }
    while(gyroValue < minGyro+352){
        gyroValue = sensor_get_value(0, gyroSensor, 0);
        tacho_run_forever(MOTOR_BOTH);
    }
    tacho_stop(MOTOR_BOTH);
    tacho_set_speed_sp(MOTOR_BOTH, max_speed_big*0.6);
    tacho_run_forever(MOTOR_BOTH);
    Sleep(5000);
    tacho_stop(MOTOR_BOTH);
///////////////////////////////////
/*
    tacho_set_speed_sp(MOTOR_LEFT, max_speed_big*0.4);
    tacho_set_speed_sp(MOTOR_RIGHT, max_speed_big*-0.4);
    float minSonic = 2000;
    do{
        TouchReturnValue = sensor_get_value(0, touchSensor, 0);
        tacho_run_forever(MOTOR_BOTH);
        sonicValue = sensor_get_value(0, sonicSensor, 0);
        printf("minSonic: %f        SonicValue: %f \n", minSonic, sonicValue);
        if(sonicValue < minSonic && sonicValue > 300){
            minSonic = sonicValue;
        }
        else if(sonicValue-5 > minSonic && sonicValue < 550){
            tacho_set_speed_sp(MOTOR_BOTH, max_speed_big*0.4);
            tacho_run_forever(MOTOR_BOTH);
            Sleep(3500);
            tacho_stop(MOTOR_BOTH);
            break;
        }
    }while(!TouchReturnValue);


*/
///////////////////////////////////
/*
    int boolean = 1;
    while(boolean == 1){
        sonicValue = sensor_get_value0(sonicSensor, 0);
        if(sonicValue > distance){
            tacho_set_speed_sp(MOTOR_LEFT, max_speed_big*0.4);
            tacho_set_speed_sp(MOTOR_RIGHT, max_speed_big*-0.4);
            tacho_run_forever(MOTOR_BOTH);
        }
        else if(sonicValue <= distance && sonicValue > 200){
            tacho_set_speed_sp(MOTOR_BOTH, max_speed_big*0.4);
            tacho_run_forever(MOTOR_BOTH);
        }
        else{
            Sleep(3500);
            tacho_stop(MOTOR_BOTH);
            boolean = 0;
        }
    }
*/
}

void find_wall(int distance){

    tacho_set_speed_sp(MOTOR_BOTH, max_speed_big*0.4);
    sonicValue = sensor_get_value0(sonicSensor, 0);
    while(sonicValue > distance){
        sonicValue = sensor_get_value0(sonicSensor, 0);
        tacho_run_forever(MOTOR_BOTH);
    }
    tacho_stop(MOTOR_BOTH);
}

void go_forward(int distance, float speed){

    tacho_set_speed_sp(MOTOR_BOTH, max_speed_big*speed);
    tacho_run_forever(MOTOR_BOTH);
    Sleep((distance*20)/speed);
    tacho_stop(MOTOR_BOTH);
    //gyr

}

void go_backwards(int distance, float speed){

    tacho_set_speed_sp(MOTOR_BOTH, max_speed_big*-speed);
    tacho_run_forever(MOTOR_BOTH);
    Sleep((distance*20)/speed);
    tacho_stop(MOTOR_BOTH);
    //gyr

}