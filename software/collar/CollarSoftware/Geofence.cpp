#include "Geofence.h"

//Macros
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


Geofence::Geofence(){
;;
}


float Geofence::degrees2radians(float degrees){
    return degrees*M_PI/180;
}
float Geofence::sqr(float x){
  return x*x;
}


//Step 1: Equirectangular Approximation Distance Calculation Function
float Geofence::distance(struct position v, struct position w) {   //Equirectangular Approximation
  float p1 = degrees2radians(v.lon - w.lon)* cos( 0.5*degrees2radians(v.lat+w.lat) ) ;
  float p2 = degrees2radians(v.lat - w.lat);
  return 6371000 * sqrt( p1*p1 + p2*p2);
} 


//Step 2: Test if the point is within the polygon of points
bool Geofence::pointInPolygon(struct position me, struct position fencePt[], int Corners) {
  //Based on http://alienryderflex.com/polygon/
  //oddNodes = 1 means within the polygon, oddNodes = 0 outside the polygon.
  int   i, j=Corners-1 ;
  bool  oddNodes=0;
  float temp=0;
  int sidebehind;
  struct position projection,projectionmax;

  for(i=0; i<Corners; i++) {		
    if(((fencePt[i].lat< me.lat && fencePt[j].lat>=me.lat) 
      || (fencePt[j].lat< me.lat && fencePt[i].lat>=me.lat))  
      &&  (fencePt[i].lon<=me.lon || fencePt[j].lon<=me.lon)) {
      oddNodes^=(fencePt[i].lon+(me.lat-fencePt[i].lat) / 
        (fencePt[j].lat-fencePt[i].lat)*(fencePt[j].lon-fencePt[i].lon)<me.lon); 
    }
    j=i; 
  }

  return oddNodes; //True Point is within the boundary, False then outside
}

//Step 3: Find which sides of the boundary we are outside
float Geofence::distBehind(struct position me, struct position w, struct position v){
  //Returns a negative if outside that boundary.
  float Fplat =w.lat;
  float Fplon =w.lon;
  //Calculate the unit length normal vector: Fn
  float Fnlat =  w.lon-v.lon;       // y' 
  float Fnlon = - (w.lat-v.lat);    //-x'
  float mag=sqrt(sqr(Fnlat)+sqr(Fnlon));
  Fnlat /= mag;           //Unit length
  Fnlon /= mag;
  //me-Fp
  Fplat = me.lat- Fplat;   //Reuse variables
  Fplon = me.lon - Fplon;
  //Return the dot product
  return Fplat*Fnlat + Fplon*Fnlon;
}

position Geofence::findProjection(struct position me, struct position v, struct position w){
  position projection;
  //Check if the two side points are in the same location (avoid dividing by zero later)
  float l = distance(v,w);
  if(l==0) return v;
  //Find the max and min x and y points
  float minx = MIN(v.lat, w.lat);
  float maxx = MAX(v.lat, w.lat);
  float miny = MIN(v.lon, w.lon);
  float maxy = MAX(v.lon, w.lon);

  //struct position projection;
  if(me.lat<minx && me.lon<miny){         //me does not fall between the two points and is closest to the lower corner
    projection.lat = minx; 
    projection.lon = miny; 
  }else if(me.lat>maxx && me.lon>maxy){   //me does not fall between the two points and is closest to the lower corner
    projection.lat = maxx; 
    projection.lon = maxy; 
  }else{                                //me does fall between the two points, project point onto the line
    float x1=v.lat, y1=v.lon, x2=w.lat, y2=w.lon, x3=me.lat, y3=me.lon;
    float px = x2-x1, py = y2-y1, dAB = px*px + py*py;
    float u = ((x3 - x1) * px + (y3 - y1) * py) / dAB;
    float x = x1 + u * px, y = y1 + u * py;
    projection.lat = x; 
    projection.lon = y; 
  }
  return projection;
}

// //Step 4: Get an accurate shortest distance to a side of the fence
// float Geofence::dist2segment(struct position v, struct position w){
//   //Check if the two side points are in the same location (avoid dividing by zero later)
//   float l = distance(v,w);
//   if(l==0) return distance(me,v);
//   //Find the max and min x and y points
//   float minx = MIN(v.lat, w.lat);
//   float maxx = MAX(v.lat, w.lat);
//   float miny = MIN(v.lon, w.lon);
//   float maxy = MAX(v.lon, w.lon);

//   //struct position projection;
//   if(me.lat<minx && me.lon<miny){         //me does not fall between the two points and is closest to the lower corner
//     projection->lat = minx; 
//     projection->lon = miny; 
//   }else if(me.lat>maxx && me.lon>maxy){   //me does not fall between the two points and is closest to the lower corner
//     projection->lat = maxx; 
//     projection->lon = maxy; 
//   }else{                                //me does fall between the two points, project point onto the line
//     float x1=v.lat, y1=v.lon, x2=w.lat, y2=w.lon, x3=me.lat, y3=me.lon;
//     float px = x2-x1, py = y2-y1, dAB = px*px + py*py;
//     float u = ((x3 - x1) * px + (y3 - y1) * py) / dAB;
//     float x = x1 + u * px, y = y1 + u * py;
//     projection->lat = x; 
//     projection->lon = y; 
//   }
//   //Return the distance to the closest point on the line.
//   return distance(me, *projection);
// }

//Bearing from the closest Point on fence to the collar
float Geofence::bearing2fence(struct position me, struct position projection){
  float y = sin(me.lon-projection.lon) * cos(me.lat);
  float x = cos(projection.lat)*sin(me.lat) - sin(projection.lat)*cos(me.lat)*cos(me.lon-projection.lon);
  float brng = atan2(y, x)*180/M_PI;
  // if (brng>0)
  // {
  //   brng=brng-2*M_PI;
  // }
  // brng=2*M_PI+brng;
  return brng;
}

fenceProperty Geofence::geofence(struct position me, struct position fencePt[], int Corners){
  bool inside = pointInPolygon(me, fencePt, Corners);

  float temp=0;
  position projection, tempprojection;
  fenceProperty result;
  result.distance=0; result.sideOutside=0; result.bearing=0;

  if(!inside){
    if( distBehind(me,fencePt[Corners-1],fencePt[0]) < 0) {
      projection=findProjection(me, fencePt[Corners-1],fencePt[0]);
      result.distance=distance(me, projection);
      result.sideOutside=0;
    }
    for(int i=1; i<Corners; i++){ 
      if( distBehind(me,fencePt[i-1],fencePt[i]) < 0 ) {
        tempprojection=findProjection(me, fencePt[i-1],fencePt[i]);
        temp=distance(me, tempprojection);
        if (temp > result.distance) {
          result.distance = temp; //if further away from side update the max distance behind
          result.sideOutside=i;
          projection=tempprojection;
        }
      }
    }

  //Determine left or right hand side alert
    result.bearing = bearing2fence(me, projection);
  //Get Compass bearing, determine left of right
  } 
  return result;
}



