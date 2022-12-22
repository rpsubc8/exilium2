/**********************************************/
/* Programa que utiliza el Joystick con SDL   */
/*                                            */
/* Autor: Jaime Jose Gavin Sierra   Alias: JJ */
/*                                            */
/* compilar: gcc -s -O9 joy.c -ojoy.exe -lsdl */
/*                                            */
/* Requiere WIN32 y MINGW32                   */
/**********************************************/

#include <stdlib.h>
#include <windows.h>
#include <SDL\SDL.H>


SDL_Surface *screen;
SDL_Joystick * joy;
int numeroJoystick,i, coord_x, coord_y;

int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    PSTR szCmdLine,
                    int iCmdShow) {

 if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
     fprintf(stderr,"No se puede inicializar las SDL: %s\n", SDL_GetError());
     exit(1);
 }
 else{
  screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE);
  if ( screen == NULL ) {
      fprintf(stderr, "No puedo poner 640x480: %s\n", SDL_GetError());
      exit(1);
  }
  else{
   SDL_InitSubSystem(SDL_INIT_JOYSTICK);
   numeroJoystick = SDL_NumJoysticks();
   printf ("N£mero de joysticks: %d\n",numeroJoystick);

   for (i=0;i<numeroJoystick;i++)
    printf ("Nombre %s\n",SDL_JoystickName(i));

   if (numeroJoystick>0){
    joy = SDL_JoystickOpen(0);

    if (joy){
     printf("Joystick abierto con ‚xito\n");

     coord_x = SDL_JoystickGetAxis (joy,0); //El 0 es las X
     coord_y = SDL_JoystickGetAxis (joy,1); //El 1 es la Y

     printf ("coordenada X: %d\nCoordenada Y: %d\n",coord_x,coord_y);
     SDL_JoystickClose(0);
    }
   }
  }
 }
 atexit(SDL_Quit);
}
