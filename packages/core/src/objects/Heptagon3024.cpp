#include "Heptagon3024.h"

void Heptagon3024::setup() {
    Model::maxWeights = 7 * 4 * 2;

    // todo: set maxLength for all models
    addModel(new Model(M_DEFAULT, 10, GROUP1));
    addModel(new Model(M_STAR, 0, GROUP1));
    addModel(new Model(M_OUTER_SUN, 10, GROUP1, 602));
    addModel(new Model(M_INNER_TRIS, 0, GROUP2));
    addModel(new Model(M_SMALL_STAR, 0, GROUP2));
    addModel(new Model(M_INNER_SUN, 0, GROUP2));
    addModel(new Model(M_SPLATTER, 10, GROUP2 | GROUP3));

    // outer (inter[0])
    addBridge(3023, 0, GROUP1); // bottom
    addBridge(2159, 2160, GROUP1); // bottom left
    addBridge(1295, 1296, GROUP1); // left
    addBridge(431, 432, GROUP1); // top left
    addBridge(2591, 2592, GROUP1); // top right
    addBridge(1727, 1728, GROUP1); // right
    addBridge(863, 864, GROUP1); // bottom right

    // middle (inter[1])
    addIntersection(new Intersection(4, 2014, 138, GROUP2));  // bottom left
    addIntersection(new Intersection(4, 2292, 1142, GROUP2)); // left
    addIntersection(new Intersection(4, 1428, 267, GROUP2));  // upper left
    addIntersection(new Intersection(4, 2443, 566, GROUP2));  // top
    addIntersection(new Intersection(4, 2752, 1573, GROUP2));  // upper right
    addIntersection(new Intersection(4, 1869, 717, GROUP2));  // right
    addIntersection(new Intersection(4, 2883, 1001, GROUP2));  // bottom right

    for (uint8_t i=0; i<7; i++) {
      Intersection *outerNeuron1 = inter[0][i*2+1];
      Intersection *outerNeuron2 = inter[0][((i+1)*2) % 14];
      addConnection(new Connection(outerNeuron1, inter[1][i], GROUP2));
      addConnection(new Connection(outerNeuron2, inter[1][i], GROUP2));
    }

    // inner (inter[2)
    addIntersection(new Intersection(4, 1964, 1053, GROUP3)); //bottom
    addIntersection(new Intersection(4, 181, 1099, GROUP3)); // bottom left
    addIntersection(new Intersection(4, 2330, 227, GROUP3)); // left
    addIntersection(new Intersection(4, 2376, 1474, GROUP3)); // top left
    addIntersection(new Intersection(4, 1520, 628, GROUP3)); // top right
    addIntersection(new Intersection(4, 2790, 678, GROUP3)); // right
    addIntersection(new Intersection(4, 2841, 1915, GROUP3)); // bottom right

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
    
  addGap(0, 143); // bottom right
  addGap(287, 431); // top left
  addGap(864, 935); // right
  addGap(1223, 1295); // left
  addGap(2016, 2735); // large left (starts bottom left)
  addGap(2880, 3023); // bottom right
}
