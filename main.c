#include "main.h"
#include "RP6RobotBaseLib.h"

enum state RP6State;
Settings_t RP6Settings;

int main(void)
{
    RP6State = IDLE;
    
    //set initial settings
    RP6Settings.driveDirection = FWD;
    RP6Settings.rotateDirection = LEFT;
    RP6Settings.speed = 0;
    RP6Settings.rotate = FALSE;
    RP6Settings.driveToStart = FALSE;
    RP6Settings.driveToTarget = FALSE;
    RP6Settings.movementComplete = TRUE;
    RP6Settings.distanceToTarget = DIST_M(1);
    RP6Settings.distanceToStart = 0;

    //stopwatch will be needed to prevent multiple bumper events to fire after IDLE state
    startStopwatch1();

    initRobotBase();
    powerON();

    //main loop
    while(1)
    {
        //the state machine waits for evenst and changes its state and the RP6Settings accordingly
        stateMachine();
        //evalSettings interprets the RP6Settings and makes the commands for movement and rotation
        evalSettings();

        task_RP6System();
    }

    return 0;

}

void stateMachine()
{
    switch(RP6State)
    {
    case IDLE:
        if(RP6Settings.movementComplete)
        {
            //DEBUG
            writeString("IDLE \n");

            if(bumper_left)
            {
                //Stopwatch is set to 0
                setStopwatch1(0);
                RP6Settings.driveDirection = FWD;
                RP6Settings.rotateDirection = LEFT;
                RP6Settings.speed = 60;
                RP6Settings.rotate = FALSE;
                RP6Settings.driveToTarget = TRUE;

                RP6State = DRIVE_TO_TARGET;
            }
            else if(bumper_right)
            {
                //Stopwatch is set to 0
                setStopwatch1(0);
                RP6Settings.driveDirection = FWD;
                RP6Settings.rotateDirection = RIGHT;
                RP6Settings.speed = 60;
                RP6Settings.rotate = FALSE;
                RP6Settings.driveToTarget = TRUE;

                RP6State = DRIVE_TO_TARGET;
            }
        }
        break;

    case DRIVE_TO_TARGET:

        writeString("DRIVE TO TARGET \n");
        
        //is true if the robt has driven the full distance of 1 metre
        if(RP6Settings.movementComplete)
        {
            writeString("ROTATE BECAUSE OF MOVE COMPLETE \n");
            RP6Settings.rotate = TRUE;
            RP6Settings.driveToTarget = FALSE;
            RP6Settings.driveToStart = TRUE;
            RP6Settings.distanceToStart = DIST_M(1);
            RP6Settings.distanceToTarget = 0;

            RP6State = ROTATE;
        }
        //is true if the left bumper got pressed before the robot reached its destination
        //there has to be one second after the switch from the IDLE state, so we prevent to fire this immediatly
        //this can only happen after the IDLE state
        else if(bumper_left && getStopwatch1() > 1000)
        {
            writeString("ROTATE BECAUSE OF BUMPER-PRESS \n");
            RP6Settings.movementComplete = TRUE;
            RP6Settings.rotate = TRUE;
            RP6Settings.driveToTarget = FALSE;
            RP6Settings.driveToStart = TRUE;
            RP6Settings.rotateDirection = LEFT;
            //the distance to start is now the distance we already drove
            RP6Settings.distanceToStart += getLeftDistance();
            //we can calculate this distance through the first one and the given distance of 1 metre
            RP6Settings.distanceToTarget =  DIST_M(1) - RP6Settings.distanceToStart;

            RP6State = ROTATE;
        }
        //is true if the right bumper got pressed before the robot reached its destination
        //there has to be one second after the switch from the IDLE state, so we prevent to fire this immediatly
        //this can only happen after the IDLE state
        else if(bumper_right && getStopwatch1() > 1000)
        {
            writeString("ROTATE BECAUSE OF BUMPER-PRESS \n");
            RP6Settings.movementComplete = TRUE;
            RP6Settings.rotate = TRUE;
            RP6Settings.driveToTarget = FALSE;
            RP6Settings.driveToStart = TRUE;
            RP6Settings.rotateDirection = RIGHT;
            //the distance to start is now the distance we already drove
            RP6Settings.distanceToStart += getLeftDistance();
            //we can calculate this distance through the first one and the given distance of 1 metre
            RP6Settings.distanceToTarget =  DIST_M(1) - RP6Settings.distanceToStart;

            RP6State = ROTATE;
        }
        break;

    case DRIVE_TO_START:
        //DEBUG
        writeString("DRIVE TO START \n");
        //is true if the robot has returned to his starting point
        if(RP6Settings.movementComplete)
        {
            RP6Settings.rotate = TRUE;
            RP6Settings.driveToTarget = FALSE;
            RP6Settings.driveToStart = FALSE;
            RP6Settings.distanceToTarget =  DIST_M(1);
            RP6Settings.distanceToStart = 0;

            RP6State = ROTATE;
        }
        //is true if the left bumper got pressed before the robot reached its starting point
        else if(bumper_left)
        {
            RP6Settings.movementComplete = TRUE;
            RP6Settings.rotate = TRUE;
            RP6Settings.driveToStart = FALSE;
            RP6Settings.driveToTarget = TRUE;
            RP6Settings.rotateDirection = LEFT;
            //the new distance to start is now the old distance minus the distance we drove during this state
            RP6Settings.distanceToStart =  RP6Settings.distanceToStart - getLeftDistance();
            RP6Settings.distanceToTarget =  DIST_M(1) - RP6Settings.distanceToStart;

            RP6State = ROTATE;

        }
        //is true if the right bumper got pressed before the robot reached its starting point
        else if(bumper_right)
        {
            RP6Settings.movementComplete = TRUE;
            RP6Settings.rotate = TRUE;
            RP6Settings.driveToStart = FALSE;
            RP6Settings.driveToTarget = TRUE;
            RP6Settings.rotateDirection = RIGHT;
            //the new distance to start is now the old distance minus the distance we drove during this state
            RP6Settings.distanceToTarget =  RP6Settings.distanceToStart - getLeftDistance();
            RP6Settings.distanceToStart = DIST_M(1) - RP6Settings.distanceToStart;

            RP6State = ROTATE;
        }
        break;

    case ROTATE:
        if(RP6Settings.movementComplete)
        {
            //DEBUG
            writeString("ROTATE \n");
            RP6Settings.rotate = FALSE;
            //after the rotation completes - change the state depending on settings set in the previous state
            if(RP6Settings.driveToStart)
            {
                RP6State = DRIVE_TO_START;
            }
            else if(RP6Settings.driveToTarget)
            {
                RP6State = DRIVE_TO_TARGET;
            }
            else if (!RP6Settings.driveToStart && !RP6Settings.driveToTarget)
            {
                RP6State = IDLE;
            }
        }

        break;
    }
}

void evalSettings()
{

    if(RP6Settings.driveToTarget && RP6Settings.movementComplete && !RP6Settings.rotate)
    {
        writeString("drive to target now\n");
        move(RP6Settings.speed, RP6Settings.driveDirection, RP6Settings.distanceToTarget, NON_BLOCKING);
        RP6Settings.movementComplete = FALSE;
    }
    else if(RP6Settings.driveToStart && RP6Settings.movementComplete && !RP6Settings.rotate)
    {
        writeString("drive to start now\n");

        move(RP6Settings.speed, RP6Settings.driveDirection, RP6Settings.distanceToStart, NON_BLOCKING);
        RP6Settings.movementComplete = FALSE;
    }
    else if(RP6Settings.rotate && RP6Settings.movementComplete)
    {
        writeString("rotate now\n");
        rotate(RP6Settings.speed, RP6Settings.rotateDirection, 180, NON_BLOCKING);
        RP6Settings.movementComplete = FALSE;
    }

    if(isMovementComplete())
    {
        writeString("Movement complete\n");
        RP6Settings.movementComplete = TRUE;
    }

}
