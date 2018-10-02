

enum state {IDLE, DRIVE_TO_TARGET, DRIVE_TO_START, ROTATE};

struct Settings_s{

    int driveDirection;
    int rotateDirection;
    int speed;
    int rotate;
    int driveToTarget;
    int driveToStart;
    int distanceToStart;
    int distanceToTarget;

    int movementComplete;

};

typedef struct Settings_s Settings_t;

void stateMachine();
void evalSettings();

