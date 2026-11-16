/* Presentation-only battle snapshot; no audio callback reads the game heap. */
#include <SDL3/SDL.h>
#include <string.h>
#include "spatial_audio.h"
static SDL_SpinLock sLock;
static struct {int active,listener,count,alive[10];float pos[40];Uint64 at;} sWorld;
static int sListener2=-1;
void PortAudio_SecondListener(int actor){SDL_LockSpinlock(&sLock);sListener2=actor;SDL_UnlockSpinlock(&sLock);}
void PortAudio_World(int active,int listener,int count,const float *positions,const int *alive){
    SDL_LockSpinlock(&sLock);
    sWorld.active=active&&count>0&&count<=10&&listener>=0&&listener<count;
    sWorld.listener=listener;sWorld.count=sWorld.active?count:0;sWorld.at=SDL_GetTicks();
    if(sWorld.active){memcpy(sWorld.pos,positions,count*4*sizeof(float));memcpy(sWorld.alive,alive,count*sizeof(int));}
    SDL_UnlockSpinlock(&sLock);
}
float PortAudio_Gain(int actor,int voice){
    float gain=1;SDL_LockSpinlock(&sLock);
    if(sWorld.active&&actor>=0&&actor<sWorld.count){
        const float *a=sWorld.pos+sWorld.listener*4,*b=sWorld.pos+actor*4;
        float x=a[0]-b[0],y=a[1]-b[1],z=a[2]-b[2];
        float d=x*x+y*y+z*z;
        if(sListener2>=0&&sListener2<sWorld.count){
            const float *c=sWorld.pos+sListener2*4;
            float x2=c[0]-b[0],y2=c[1]-b[1],z2=c[2]-b[2],d2=x2*x2+y2*y2+z2*z2;
            if(d2<d)d=d2;
        }
        gain=SDL_GetTicks()-sWorld.at>1500?0:Bt4_AudioFalloff(sqrtf(d),voice?8:10,voice?120:160);
    }
    SDL_UnlockSpinlock(&sLock);return gain;
}
int PortAudio_Alive(int actor){
    int alive=1;SDL_LockSpinlock(&sLock);
    if(sWorld.active&&actor>=0&&actor<sWorld.count)alive=sWorld.alive[actor];
    SDL_UnlockSpinlock(&sLock);return alive;
}
