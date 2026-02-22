#include "Heptagon919.h"

void Heptagon919::setup() {
    Model::maxWeights = 7 * 4 * 2;

    // todo: set maxLength for all models
    addModel(new Model(M_DEFAULT, 10, GROUP1));
    addModel(new Model(M_STAR, 0, GROUP1));
    addModel(new Model(M_OUTER_SUN, 10, GROUP1, 602));
    addModel(new Model(M_INNER_TRIS, 0, GROUP2));
    addModel(new Model(M_SMALL_STAR, 0, GROUP2));
    addModel(new Model(M_INNER_SUN, 0, GROUP2));
    addModel(new Model(M_SPLATTER, 10, GROUP2 | GROUP3));

    // outer GROUP1
    addBridge(918, 1, GROUP1); // bottom
    addBridge(653, 654, GROUP1); // bottom left
    addBridge(389, 390, GROUP1); // left
    addBridge(125, 126, GROUP1); // top left
    addBridge(789, 791, GROUP1); // top right
    addBridge(522, 525, GROUP1); // right
    addBridge(260, 263, GROUP1); // bottom right

    // middle GROUP2
    addIntersection(new Intersection(4, 612, 42, GROUP2));  // bottom left
    addIntersection(new Intersection(4, 696, 347, GROUP2)); // left
    addIntersection(new Intersection(4, 434, 81, GROUP2));  // upper left
    addIntersection(new Intersection(4, 742, 172, GROUP2));  // top
    addIntersection(new Intersection(4, 836, 478, GROUP2));  // upper right
    addIntersection(new Intersection(4, 568, 218, GROUP2));  // right
    addIntersection(new Intersection(4, 876, 304, GROUP2));  // bottom right

    for (uint8_t i=0; i<7; i++) {
      Intersection *outerNeuron1 = inter[0][i*2+1];
      Intersection *outerNeuron2 = inter[0][((i+1)*2) % 14];
      addConnection(new Connection(outerNeuron1, inter[1][i], GROUP2));
      addConnection(new Connection(outerNeuron2, inter[1][i], GROUP2));
    }

    // inner GROUP3
    addIntersection(new Intersection(4, 597, 320, GROUP3)); //bottom
    addIntersection(new Intersection(4, 55, 334, GROUP3)); // bottom left
    addIntersection(new Intersection(4, 708, 69, GROUP3)); // left
    addIntersection(new Intersection(4, 722, 448, GROUP3)); // top left
    addIntersection(new Intersection(4, 462, 191, GROUP3)); // top right
    addIntersection(new Intersection(4, 848, 206, GROUP3)); // right
    addIntersection(new Intersection(4, 863, 582, GROUP3)); // bottom right

    // models
    for (uint8_t i=0; i<7; i++) {
        Intersection *middleNeuron1 = inter[1][i];
        Intersection *middleNeuron2 = inter[1][(i-1+7) % 7];
        Connection *middleConnection1 = addConnection(new Connection(middleNeuron1, inter[2][i], GROUP3));
        Connection *middleConnection2 = addConnection(new Connection(middleNeuron2, inter[2][i], GROUP3));
        models[M_STAR]->put(middleConnection1->fromPort, middleNeuron1->ports[1], 10);
        models[M_STAR]->put(middleConnection2->fromPort, middleNeuron2->ports[0], 10);
        models[M_OUTER_SUN]->put(middleConnection1, 0);
        models[M_OUTER_SUN]->put(middleConnection2, 0);
        models[M_INNER_TRIS]->put(middleConnection1, 10);
        models[M_INNER_TRIS]->put(middleConnection2, 10);
        models[M_SMALL_STAR]->put(middleConnection1->fromPort, 10);
        models[M_SMALL_STAR]->put(middleConnection2->fromPort, 10);
        models[M_INNER_SUN]->put(middleConnection1->fromPort, 10);
        models[M_INNER_SUN]->put(middleConnection2->fromPort, 10);
        models[M_INNER_SUN]->put(middleConnection1->toPort, 10);
        models[M_INNER_SUN]->put(middleConnection2->toPort, 10);
    }

    for (uint8_t i=0; i<7; i++) {
        Intersection *innerNeuron1 = inter[2][i];
        Intersection *innerNeuron2 = inter[2][(i+1) % 7];
        Connection *innerConnection = addConnection(new Connection(innerNeuron1, innerNeuron2, GROUP4));
        models[M_STAR]->put(innerConnection->fromPort, innerNeuron1->ports[1], 10);
        models[M_STAR]->put(innerConnection->toPort, innerNeuron2->ports[0], 10);
        models[M_SMALL_STAR]->put(innerConnection->fromPort, innerNeuron1->ports[1], 10);
        models[M_SMALL_STAR]->put(innerConnection->toPort, innerNeuron2->ports[0], 10);
        models[M_OUTER_SUN]->put(innerConnection, 0);
        models[M_INNER_TRIS]->put(innerConnection, 10);
    }
}
