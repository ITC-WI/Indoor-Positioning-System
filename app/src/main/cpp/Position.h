//
// Created by Auro on 29-01-2021.
//

#ifndef INDOOR_POSITIONING_SYSTEM_POSITION_H
#define INDOOR_POSITIONING_SYSTEM_POSITION_H


/**
 * Maintains the position estimate as obtained by multilateration. Can be used in fusion
 * algorithm with dead reckoning.
 */
 //TODO: Can apply singleton for this class.
class Position {
public:
    static int x;
    static int y;
};


#endif //INDOOR_POSITIONING_SYSTEM_POSITION_H
