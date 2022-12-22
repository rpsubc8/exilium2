/*****************************************************************************/
/* DISKMAG para LINUX                                                        */
/* Compila: gcc -s -O6 demo.c -olinuxexilium `allegro-config --libs` -ljgmod */
/* Hay que cambiar la libreria alleg-4.0.1.so por alleg.so o bien            */
/* especificar ese nombre al compilar                                        */
/* Nota: El jgmod va despues de las allegro, ya que sino no suena sonido     */
/* Autor: Jaime Jose Gavin Sierra.  Alias: J.J.                              */
/*****************************************************************************/

//Para mayor velocidad de compilacion
#define alleg_flic_unused
#define alleg_math_unused
#define alleg_gui_unused


#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "config.h"
#include "datos.h"
#include "sliders1.h"
#ifdef ALLEGRO_DOS
 #include "optimiza.c"
#endif


#define SETUP_EMBEDDED        //Para el SETUP
#define SETUP_TITLE "Configuracion EXILIUM"
#define SETUP_CFG_FILE "allegro.cfg"
#include "SETUP\SETUP.C"

#define maxFotos 20           //Soporte para 20 fotos cada articulo
#define maxInfo 3100          //Con soporte para mas de 100 hojas 100*30=3000
#define maxArticulos 50       //Soporte para 50 articulos
#define maxItem 100           //Soporte para 100 Items por linea
#define longitud_cadena 79    //Longitud maxima de la cadena de texto
#define desX_cadena 4         //Desplazamiento x de la cadena en pantalla
#define mas 0
#define menos 1
#define anchoX 640
#define altoY 480



/*******************************/
/* id 0 texto normal           */
/* id 1 texto color rojo       */
/* id 2 texto color negro      */
/*******************************/

typedef unsigned char boolean;

//enum TitemColor{color_rojo,color_verde,color_azul,color_negro,color_blanco,color_marron,color_amarillo,color_violeta,color_verdefosforito,color_azulclaro,color_naranja,color_gris,color_grisclaro};
enum TitemJustificacion{derecha,izquierda,centro,nojustificado};
//enum TitemLetra{letra_grande,letra_normal,letra_pc,letra_pct,letra_xm,letra_xmb,letra_xmi};

typedef struct{  //Dibujo el bloque a partir de y + 10 pixels
 unsigned char xIni; //La coordenada X de inicio a dibujar
 short int yIni; //La coordenada Y de inicio a dibujar
 short int numFoto;   //El indice a la foto. 0 no hay foto
}TFoto;

typedef struct{
// enum TitemColor color;
//enum TitemLetra letra;
 int color;           //Contiene el color en crudo para optimizar
 short int letra;           //Contiene el indice a la fuente para optimizar
 enum TitemJustificacion justificacion;
 TFoto laFoto; //Un puntero ocupa 4 bytes, y esta estructura 4 bytes
}TitemDatos;

typedef struct{
 int color;
 short int letra;
 char * cadena;
 struct Ttexto * sig;
}Ttexto;

typedef struct{
 TitemDatos item;
 char * cadena;
 Ttexto * sig;
}Tnodo;

typedef struct{
 unsigned char numFotos;      //El numero de fotos del documento
 short int numFilas;                //El numero de filas del documento
 short int posFila;                 //La fila actual del documento
 Tnodo info[maxInfo];
// char *texto[maxInfo];
 BITMAP * fotos[maxFotos];    //Lista de todas las fotos del documento
 PALETTE * paletas[maxFotos]; //Lista de las paletas de cada foto
}TArticulo;                   //Contiene todo el articulo

typedef struct{               //Contiene a los articulos por menu de inicio
 unsigned char numArticulos;  //El numero total de articulos
 unsigned char articuloActual;//El articulos actual
 DATAFILE * articulo[maxArticulos]; //El indice al articulo
 char *textoPlataForma[maxArticulos];  //El nombre de la plataforma
 char *textoAutor[maxArticulos];       //El nombre del autor del articulo
 char *textoArticulo[maxArticulos];    //El titulo del articulo
 char *textoWeb[maxArticulos];         //La pagina web
 char *textoCorreo[maxArticulos];      //El correo electronico
}TArticulos;


boolean menuArticulosDesplegado = FALSE;  //Para desplegar menu de articulos
boolean menuBuscar = FALSE;               //Para sacar menu Buscar
boolean parametroSetup = FALSE;           //Para el setup
boolean parametroNo = FALSE;
boolean parametro8 = FALSE;
boolean palancaJuegos = FALSE;            //Para el joystick
volatile crono=0;
MIDI *archivoMidi;                        //El archivo de sonido MIDI
short int numMidiActual;                  //El numero actual de MIDI
short int numVolumenActual=230;           //El volumen actual
short int i;
boolean sonido=FALSE,raton=FALSE,driverSonido=FALSE; //GLOBAL para saber si hay sonido
PALETTE paletaGris;
TArticulo elArticulo; //GLOBAL Aqui guardo el Articulo
BITMAP *fondo = NULL;
BITMAP *letras = NULL;
BITMAP *videoTemporal = NULL;
BITMAP *auxBitmap = NULL;
DATAFILE *datafile; //Contiene los datos de FUENTES y demas de la revista
DATAFILE *revista;  //Contiene los articulos de la revista
DATAFILE *configuracion; //Contiene los datos de configuracion y fotos
TArticulos losArticulos;
short int grises;         //Contiene si estamos en gris o color
int color_rojo,color_verde,color_azul,color_negro,color_blanco,color_marron,color_amarillo,color_violeta; //Para optimizar
int color_verdeFosforito,color_azulClaro,color_naranja,color_gris,color_grisClaro;
char cadenaClave[255];   //aqui tenemos la clave


void ScrollArriba();
void ScrollAbajo();
void ScrollArriba30Lineas();
void ScrollAbajo30Lineas();
void LeerArchivoDatos(char *nombre, DATAFILE **datos);
void BuscarItem(char *cad, TitemDatos * itemDatos);
void MostrarArticulo(DATAFILE * indice);
void InicializarArticulo();
void EliminarArticulo();
void InicializarLosArticulos(TArticulos *losArticulos);
void EliminarLosArticulos(TArticulos *losArticulos);
void LeerArticulos(TArticulos *articulos, DATAFILE *revista);
void MostrarArticulosPantalla(TArticulos *losArticulos, BITMAP *pantalla);
void ScrollArribaArticulos(TArticulos *losArticulos, BITMAP *pantalla);
void ScrollAbajoArticulos(TArticulos *losArticulos, BITMAP *pantalla);
void ControlaEventos();
void MostrarTitulo(BITMAP *pantalla);
void MostrarInfoArticulo(BITMAP *pantalla);
void MostrarBotones();
//int JustificacionIzquierda(char *cadena,enum TitemLetra tipoLetra);
int JustificacionDerecha (char *cadena,int tipoLetra);
void AccionSonido();
void SiguienteCancion();
void HacerJustificacion(char *cadena,int tipoLetra,enum TitemJustificacion justificacion, short int total);
void GeneraColores();
void Presentacion(DATAFILE *datos);
int ActivaMOD();
void DestruyeMOD();
void inc_crono1();
void Fundir();
void Desfundir(PALETTE paleta);
void Despedida(DATAFILE *datos);
void ReguladorVolumen(unsigned char tipo);
void MostrarLineaComandos(void);
void GeneraClave(char *cadena);
void AccionMenuDesplegable(void);
void AccionBarraDesplegable(void);
void ControlaEventoRaton(int *ratonX, int *ratonY, int *ratonZantes, boolean *salir);
void ControlaEventoTeclado(boolean * salir);
void ControlaEventoJoystick();
boolean ComparaCadena(char *cad1, char *cad2);
void AccionBuscar(boolean *salir);
//void LeerArticuloDisco(char *nombre,TArticulo articulo);


int main(int argc,char *argv[]){
 if (argc>1){
  if ((strcmp("/?",argv[1])==0)||(strcmp("?",argv[1])==0)){
    MostrarLineaComandos();
    return (0);
  }
 }

 allegro_init();
 
 set_uformat(U_ASCII);
 
 install_keyboard();      //Instalo el teclado


 //install_timer();
 if (install_mouse()!=-1) raton=1; //Instalo el raton
 install_timer();         //Para poder mostrar raton activo el timer

 //Aqui estan los archivos con password odioaloslamers

 set_gfx_mode(GFX_SAFE,640,480,0,0);

 if (install_joystick(JOY_TYPE_AUTODETECT)==0){
   if (!num_joysticks)  palancaJuegos = FALSE;
   else  palancaJuegos = TRUE;  //Instalo el joystick
 }
 else
   palancaJuegos = FALSE;


 for (i=0;i<argc;i++){
  if ((stricmp(argv[i],"setup")==0)||(stricmp(argv[i],"SETUP")==0))
    parametroSetup = TRUE;
  if ((stricmp(argv[i],"no")==0)||(stricmp(argv[i],"NO")==0))
    parametroNo = TRUE;
  if (stricmp(argv[i],"8")==0)
    parametro8 = TRUE;  
 }

 if (parametroSetup) setup_main();
 
 GeneraClave(cadenaClave);
 packfile_password(cadenaClave);
  LeerArchivoDatos("datos.dat",&datafile);
  LeerArchivoDatos("sliders1.dat",&revista);
  LeerArchivoDatos("config1.dat",&configuracion);
 packfile_password(NULL);

 set_color_conversion(COLORCONV_NONE);


 if (parametroNo==FALSE) Presentacion(configuracion);
 if (parametro8){
  set_color_depth(8);
  grises = TRUE; 
 }
 else{
  set_color_depth(16);
  grises = FALSE; //Estamos en color
 }

 
 letras = create_bitmap(640,480);
 auxBitmap = create_bitmap(640,480);
 videoTemporal = create_bitmap(640,480);
 clear_to_color(letras,bitmap_mask_color(letras));
 if (set_gfx_mode(GFX_AUTODETECT,640,480,0,0)!=0){
  allegro_message("Error en modo de video\nCambie driver grafico en CFG\nO pruebe en escala de grises");
  allegro_exit();
  printf("Error en modo de video\nCambie driver grafico en CFG\nO pruebe en escala de grises");
  exit(-1);
 }



 //Peparamos la pelata de grises
 if (grises){
  for (i=0;i<256;i++) paletaGris[i].r = paletaGris[i].g = paletaGris[i].b = (i>>2);
  set_palette(paletaGris);            //Ponemos la paleta de grises
 }

 GeneraColores();
 
 if (raton) show_mouse(screen);                       //Muestro raton
 fondo = configuracion[FOTO_FONDO].dat;
// set_palette(configuracion[PALETA_FONDO].dat);
// hline(fondo,0,80,639,makecol(255,255,255));  //Lineas separatorias
// hline(fondo,0,400,639,makecol(255,255,255));


 InicializarLosArticulos(&losArticulos);
 LeerArticulos(&losArticulos, revista);
 
 InicializarArticulo();
 ActivaMOD();
 MostrarArticulo(losArticulos.articulo[0]);  //Muestra el articulo de delabu
 ControlaEventos();                           //Se mete en bucle de eventos



 if (driverSonido!=FALSE) DestruyeMOD();


 if (parametroNo==FALSE) Despedida(configuracion);  
 
 //set_gfx_mode(GFX_TEXT,80,25,0,0);
/* for (i=0;i<10;i++){
  printf("%d ",elArticulo.info[i].item.color);
 }
 printf("\nNumero de fotos totales: %d\nArgumentos: %d\n",elArticulo.numFotos,argc);
*/
 EliminarArticulo();
 EliminarLosArticulos(&losArticulos);
 
/* set_gfx_mode(GFX_TEXT,80,25,0,0);
 for (i=78;i<100;i++){
  printf("%d %c ",((char *)datafile[TEXTO_REVISTA].dat)[i],((char *)datafile[TEXTO_REVISTA].dat)[i]);
 }*/


 unload_datafile(configuracion);
 unload_datafile(revista);
 unload_datafile(datafile);
 destroy_bitmap(auxBitmap);
 destroy_bitmap(letras);
 destroy_bitmap(videoTemporal);
 if (raton) show_mouse(NULL);
 remove_timer;
 if (palancaJuegos!=FALSE) remove_joystick();
 if (raton) remove_mouse();
 remove_keyboard();
 allegro_exit();
 //printf("Numero filas: %d \nFila actual: %d",elArticulo.numFilas,elArticulo.posFila);
 return (0);
}
END_OF_MAIN();


/************************************************/
void ScrollArriba(){
 int auxiliar;
 int i,j=0,total=0;
 Ttexto * pAux = NULL;
 short int auxLongitud;
 
 if (elArticulo.posFila<elArticulo.numFilas){
//  total = elArticulo.numFilas-elArticulo.posFila;
//  if (total>=5) total=5;
/*  if ((elArticulo.posFila+5)>(elArticulo.numFilas))
   total = elArticulo.numFilas-elArticulo.posFila;
  else  */
   total=5;
  if (!grises) select_palette(configuracion[PALETA_FONDO].dat);
//  blit(fondo,videoTemporal,0,0,0,0,640,480);
//  blit(letras,letras,0,140,0,90,640,300);
  blit(fondo,videoTemporal,0,0,0,80,640,320);
  //rectfill(videoTemporal,0,400,640,400,0);
  blit(letras,letras,0,140,0,90,640,260);
  rectfill(letras,0,350,639,400,bitmap_mask_color(letras));


  //Dibujo las 4 filas anteriores para quitar defectos visuales
  auxiliar = 290;
  for (i=(elArticulo.posFila+1-4);i<(elArticulo.posFila+total+1);i++,j++){
   if (elArticulo.info[i].cadena!=NULL){

    //auxiliar = 300+(j<<3)+(j<<1);
    auxiliar = auxiliar+10;
    switch (elArticulo.info[i].item.justificacion){
     case nojustificado: textout(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,desX_cadena,auxiliar,elArticulo.info[i].item.color);
                         break;
     case centro: textout_centre(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,319,auxiliar,elArticulo.info[i].item.color); break;
     case izquierda: textout(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,desX_cadena,auxiliar,elArticulo.info[i].item.color);
                     break;
                     //textout_justify(letras, datafile[indiceFuente].dat,elArticulo.info[i].cadena,JustificacionIzquierda(elArticulo.info[i].cadena,elArticulo.info[i].item.letra),639,(340+j*10),640,color); break;
     case derecha: textout(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,(639-desX_cadena-JustificacionDerecha(elArticulo.info[i].cadena,elArticulo.info[i].item.letra)),auxiliar,elArticulo.info[i].item.color); break;//Soporta 80 caracteres y 30 filas
    }
    //textout(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,0,(340+(j<<3)+(j<<1)),elArticulo.info[i].item.color);

    //Pongo el resto de los items de la misma linea de texto
    if (elArticulo.info[i].sig!=NULL){
     auxLongitud = strlen(elArticulo.info[i].cadena)*8;
     pAux = elArticulo.info[i].sig;
     while (pAux!=NULL){
      textout(letras, datafile[(pAux->letra)].dat,pAux->cadena,desX_cadena+auxLongitud,auxiliar,pAux->color);
      auxLongitud = auxLongitud+(strlen(pAux->cadena))*8;
      pAux = (Ttexto *)pAux->sig;
     }
    }
    

    if ((elArticulo.info[i].item.laFoto.numFoto)!=0){
      if (!grises) select_palette(*(elArticulo.paletas[((elArticulo.info[i].item.laFoto.numFoto))]));
      blit(elArticulo.fotos[(elArticulo.info[i].item.laFoto.numFoto)],letras,0,elArticulo.info[i].item.laFoto.yIni,(((elArticulo.info[i].item.laFoto.xIni)<<3)+desX_cadena),auxiliar,elArticulo.fotos[(elArticulo.info[i].item.laFoto.numFoto)]->w,10);
    }

   }  
   //textout(letras,datafile[FUENTE_NORMAL].dat,elArticulo.texto[i],1,(340+j*10),0xFFFFFF); //Soprta 80 caracteres y 30 filas
  }
  elArticulo.posFila+=total;
  masked_blit(letras,videoTemporal,0,0,0,0,anchoX,altoY);
//  blit(videoTemporal,screen,0,0,0,0,640,480);
  MostrarInfoArticulo(letras);
  
//  if ((raton)&&(mouse_y>=30)&&(mouse_y<400)){
  if (raton){
   show_mouse(NULL);
   blit(videoTemporal,screen,0,50,0,50,anchoX,350);
   show_mouse(screen);
  }
  else blit(videoTemporal,screen,0,50,0,50,anchoX,350);
 }
}

/************************************************/
void ScrollAbajo(){
 int auxiliar;
 int i,j=0,total=0;
 Ttexto * pAux;
 short int auxLongitud;
 
 if (elArticulo.posFila>29){
//  total = elArticulo.posFila-29;
  total=5;
//  if (total>=5) total=5;
  clear_to_color(auxBitmap,bitmap_mask_color(auxBitmap));
  if (!grises) select_palette(configuracion[PALETA_FONDO].dat);
//  blit(fondo,videoTemporal,0,0,0,0,640,480);
//  blit(letras,auxBitmap,0,90,0,140,640,250);
  blit(fondo,videoTemporal,0,0,0,80,anchoX,320);
  //rectfill(videoTemporal,0,400,640,400,0);
  blit(letras,auxBitmap,0,90,0,140,anchoX,250);


  auxiliar = 80;
  for (i=(elArticulo.posFila-(total+29));i<(elArticulo.posFila-29);i++,j++)
   if (elArticulo.info[i].cadena!=NULL){

    //auxiliar = 90 + (j<<3)+(j<<1);
    auxiliar = auxiliar+10;
    switch (elArticulo.info[i].item.justificacion){
     case nojustificado: textout(auxBitmap, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,desX_cadena,auxiliar,elArticulo.info[i].item.color);
                         break;
                         //textout(auxBitmap, datafile[indiceFuente].dat,elArticulo.info[i].cadena,0,(90+j*10),color); break;
     case centro: textout_centre(auxBitmap, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,319,auxiliar,elArticulo.info[i].item.color); break;
     case izquierda: textout(auxBitmap, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,desX_cadena,auxiliar,elArticulo.info[i].item.color);
                     break;
                    // textout_justify(auxBitmap, datafile[indiceFuente].dat,elArticulo.info[i].cadena,JustificacionIzquierda(elArticulo.info[i].cadena,elArticulo.info[i].item.letra),639,(90+j*10),640,color); break;
     case derecha: textout(auxBitmap, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,(639-desX_cadena-JustificacionDerecha(elArticulo.info[i].cadena,elArticulo.info[i].item.letra)),auxiliar,elArticulo.info[i].item.color); break;//Soporta 80 caracteres y 30 filas
    }

    //Hacemos el resto de items de la misma linea de texto
    if (elArticulo.info[i].sig!=NULL){
     auxLongitud = strlen(elArticulo.info[i].cadena)*8;
     pAux = elArticulo.info[i].sig;
     while (pAux!=NULL){
      textout(auxBitmap, datafile[(pAux->letra)].dat,pAux->cadena,desX_cadena+auxLongitud,auxiliar,pAux->color);
      auxLongitud = auxLongitud+(strlen(pAux->cadena))*8;
      pAux = (Ttexto *)pAux->sig;
     }
    }
    
    //textout(auxBitmap, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,0,(90+j*10),elArticulo.info[i].item.color);

    if ((elArticulo.info[i].item.laFoto.numFoto)!=0){
      if (!grises) select_palette(*(elArticulo.paletas[((elArticulo.info[i].item.laFoto.numFoto))]));
      blit(elArticulo.fotos[(elArticulo.info[i].item.laFoto.numFoto)],auxBitmap,0,elArticulo.info[i].item.laFoto.yIni,(((elArticulo.info[i].item.laFoto.xIni)<<3)+desX_cadena),auxiliar,elArticulo.fotos[(elArticulo.info[i].item.laFoto.numFoto)]->w,10);
    }


   }  
  
//   textout(auxBitmap,datafile[FUENTE_NORMAL].dat,elArticulo.texto[i],1,(90+j*10),0xFFFFFF); //Soprta 80 caracteres y 30 filas
  elArticulo.posFila-=total;
  masked_blit(auxBitmap,videoTemporal,0,0,0,0,anchoX,altoY);
//  blit(videoTemporal,screen,0,0,0,0,640,480);
//  blit(auxBitmap,letras,0,0,0,0,640,480);
  MostrarInfoArticulo(auxBitmap);
//  if ((raton)&&(mouse_y>=30)&&(mouse_y<400)){
  if (raton){
   show_mouse(NULL);
   blit(videoTemporal,screen,0,50,0,50,anchoX,350);
   show_mouse(screen);
  }
  else blit(videoTemporal,screen,0,50,0,50,anchoX,350);
  blit(auxBitmap,letras,0,80,0,80,anchoX,320);
 }
}

/************************************************/
void LeerArchivoDatos(char *nombre, DATAFILE **datos){
 *datos = load_datafile(nombre);
 if (*datos==NULL) {
   allegro_message("Error leyendo el archivo de datos\n");
   allegro_exit();
   printf("Error leyendo el archivo de datos\n");
   exit(-1);
 }
}

/************************************************/
void BuscarItem(char *cad, TitemDatos * itemDatos){
/* [COLOR=ROJO IZQUIERDA DOBLE]*/
 int i=0,j=0,contCad=0;
 char cadenas[100][100];
 char car;
 char *endp;
 int rojo=0,verde=0,azul=0;
 
 for (i=0;i<100;i++)
  cadenas[i][0]='\0';
 
 for (i=0;i<(strlen(cad)+1);i++){
  car = cad[i];
  if ((car=='\0')||(car==' ')||(car=='=')){
   cadenas[j][contCad] = '\0';
   contCad=0;
   j++;
  }
  else{
   cadenas[j][contCad] = car;
   contCad++;
  }
 }
 for (i=0;i<j;i++){
  if (strcmp("ROJO",cadenas[i])==0) itemDatos->color = color_rojo;
  else
   if (strcmp("VERDE",cadenas[i])==0) itemDatos->color = color_verde;
   else
    if (strcmp("AZUL",cadenas[i])==0) itemDatos->color = color_azul;
    else
     if (strcmp("NEGRO",cadenas[i])==0) itemDatos->color = color_negro;
     else
      if (strcmp("BLANCO",cadenas[i])==0) itemDatos->color = color_blanco;
      else
       if (strcmp("MARRON",cadenas[i])==0) itemDatos->color = color_marron;
       else
        if (strcmp("AMARILLO",cadenas[i])==0) itemDatos->color = color_amarillo;
        else
         if (strcmp("VIOLETA",cadenas[i])==0) itemDatos->color = color_violeta;
         else
          if (strcmp("VERDEFOSFORITO",cadenas[i])==0) itemDatos->color = color_verdeFosforito;
          else
           if (strcmp("AZULCLARO",cadenas[i])==0) itemDatos->color = color_azulClaro;
           else
            if (strcmp("NARANJA",cadenas[i])==0) itemDatos->color = color_naranja;
            else
             if (strcmp("GRIS",cadenas[i])==0) itemDatos->color = color_gris;
             else
              if (strcmp("GRISCLARO",cadenas[i])==0) itemDatos->color = color_grisClaro;

  if (strcmp("COLOR",cadenas[i])==0){
    rojo = strtod(cadenas[i+1],&endp);  //Cogemos el rojo en decimal
    verde = strtod(cadenas[i+2],&endp); //Cogemos el verde en decimal
    azul = strtod(cadenas[i+3],&endp);  //Cogemos el azul en decimal
    itemDatos->color = makecol(rojo,verde,azul); //Ponemos el color
  }

  if (strcmp(cadenas[i],"GRANDE")==0) itemDatos->letra = FUENTE_GRANDE;
  if (strcmp(cadenas[i],"NORMAL")==0) itemDatos->letra = FUENTE_NORMAL;
  if (strcmp(cadenas[i],"PC")==0) itemDatos->letra = FUENTE_PC;
  if (strcmp(cadenas[i],"PCT")==0) itemDatos->letra = FUENTE_PCT;
  if (strcmp(cadenas[i],"XM")==0) itemDatos->letra = FUENTE_XM;
  if (strcmp(cadenas[i],"XMB")==0) itemDatos->letra = FUENTE_XMB;
  if (strcmp(cadenas[i],"XMI")==0) itemDatos->letra = FUENTE_XMI;
  if (strcmp(cadenas[i],"CHAR11")==0) itemDatos->letra = FUENTE_CHAR11;
  if (strcmp(cadenas[i],"CHAR11B")==0) itemDatos->letra = FUENTE_CHAR11B;
  if (strcmp(cadenas[i],"CHAR11I")==0) itemDatos->letra = FUENTE_CHAR11I;
  if (strcmp(cadenas[i],"CHAR11BI")==0) itemDatos->letra = FUENTE_CHAR11BI;
  if (strcmp(cadenas[i],"CHAR14")==0) itemDatos->letra = FUENTE_CHAR14;
  if (strcmp(cadenas[i],"CHAR14B")==0) itemDatos->letra = FUENTE_CHAR14B;
  if (strcmp(cadenas[i],"CHAR14I")==0) itemDatos->letra = FUENTE_CHAR14I;
  if (strcmp(cadenas[i],"CHAR14BI")==0) itemDatos->letra = FUENTE_CHAR14BI;
  

  if (strcmp(cadenas[i],"NO")==0) itemDatos->justificacion = nojustificado;
  if (strcmp(cadenas[i],"DERECHA")==0) itemDatos->justificacion = derecha;
  if (strcmp(cadenas[i],"IZQUIERDA")==0) itemDatos->justificacion = izquierda;  
  if (strcmp(cadenas[i],"CENTRO")==0) itemDatos->justificacion = centro;  
 }
}
/************************************************/
void MostrarArticulo(DATAFILE * indice){
 int auxiliar;
 int restoFoto;
 DATAFILE * indiceFoto, * indicePaleta;
 int columnaX,filaY;
 char *endp;
 int aux;
 double numFotos=0;
 int color=0;
 TitemDatos datosItem[maxItem];
 char cadAux[maxItem][500];
 char cadItem[500];
 int itemActual = 0;
 int contItem=0;
 int salir=0;
 int i=0;
 int j=0;
 int k=0;
 int cont=0;
 char car;
 Ttexto *pAux;
 int auxLongitud;
 short int total, resto;

 if (indice!=NULL){
  cadItem[0] = '\0';
  for (k=0;k<maxItem;k++)
   cadAux[k][0] = '\0';

  text_mode(bitmap_mask_color(letras));    //Hago que el fondo del texto sea transparente


  datosItem[0].color = color_blanco;
  datosItem[0].letra = FUENTE_PC;
  datosItem[0].justificacion = centro;
  salir=0;
  elArticulo.numFilas=0;
  i=0;
  j=0;
  cont=7;

  //Aqui leo las fotos
  cadAux[0][0]='\0';
  car=((char *)indice->dat)[cont];;
  while ((car!=']')&&(car!=10)){
   if (car!=13){
    cadAux[0][i]=car;
    i++;
   }
   cont++;
   car=((char *)indice->dat)[cont];
  }
  cadAux[0][i]='\0';
  numFotos = strtod(cadAux[0],&endp);
  elArticulo.numFotos=(int)numFotos; //Ya tengo las fotos totales
  cadAux[0][0]='\0';

  car=((char *)indice->dat)[cont];
  while (car!=10){
   cont++;
   car=((char *)indice->dat)[cont];
  }
  cont++;

 
  for (i=1;i<=numFotos;i++){//Pongo todas las fotos
   car=0;
   aux=0;
   while (car!=' '){
    car = ((char *)indice->dat)[cont];
    if ((car!=' ')&&(car!=13)){
     cadAux[0][aux]=car;
     aux++;
    }
    cont++;
   }
   cadAux[0][aux]='\0';
   //indiceFoto=(int)strtod(cadAux,&endp);
   //Cojo el indice de la foto en el dat
   indiceFoto =  find_datafile_object(revista,cadAux[0]);
   if (indiceFoto==NULL){
    allegro_message("Error en foto");
    allegro_exit();
    exit(-1);
   }
   
 // elArticulo.numFotos=(unsigned char)indiceFoto;

   car=0;
   aux=0;
   while (car!=' '){
    car = ((char *)indice->dat)[cont];
    if ((car!=' ')&&(car!=13)){
     cadAux[0][aux]=car;
     aux++;
    }
    cont++;
   }
   cadAux[0][aux]='\0';
   //indicePaleta=(int)strtod(cadAux,&endp);
   //Cojo su paleta en el dat
   indicePaleta = find_datafile_object(revista,cadAux[0]);
   if (indicePaleta==NULL){
    allegro_message("Error en paleta de foto");
    allegro_exit();
    exit(-1);
   }
   

   car=0;
   aux=0;
   while (car!=' '){
    car = ((char *)indice->dat)[cont];
    if (car!=13){
     cadAux[0][aux]=car;
     aux++;
    }
    cont++;
   }
   cadAux[0][aux]='\0';
   columnaX=(int)strtod(cadAux[0],&endp); //Su columna en el dat

   car=0;
   aux=0;
   while (car!=10){
    car = ((char *)indice->dat)[cont];
    if (car!=13){
     cadAux[0][aux]=car;
     aux++;
    }
    cont++;
   }
   cadAux[0][aux]='\0';
   filaY=(int)strtod(cadAux[0],&endp); //Su fila en el dat
   //Ahora es cuando pongo las fotos en el articulo
   elArticulo.paletas[i]= indicePaleta->dat; //revista[indicePaleta].dat;
   elArticulo.fotos[i] = indiceFoto->dat;    //revista[indiceFoto].dat;

   restoFoto = elArticulo.fotos[i]->h%10; //Controla si es > multiplos de 10
   if (restoFoto>0) restoFoto=1;          //Para coger la foto entera
  
   for (j=0;j<((elArticulo.fotos[i]->h/10)+restoFoto);j++){
    elArticulo.info[j+filaY].item.laFoto.xIni = columnaX;
    elArticulo.info[j+filaY].item.laFoto.yIni = j*10;
    elArticulo.info[j+filaY].item.laFoto.numFoto = i;
   }
  }
  cadAux[0][0]='\0';
 

 
  i=0;
  j=0;
  car=0;
 
  while (cont<(indice->size)){
   car = ((char *)indice->dat)[cont];
   if (car!=13){
    if (car=='[') {
     cont++;
     if (cont<(indice->size)){
      car = ((char *)indice->dat)[cont];
      if (car=='['){
       if (car!=13){
        cadAux[itemActual][i] = car;
        i++;
       }
      }
      else{
       contItem = 0;
       while ((car!=']')&&(car!=10)){
        if (car!=13){
         cadItem[contItem]=car;
         contItem++;
        }
        cont++;
        car = ((char *)indice->dat)[cont];
       }
       cadItem[contItem]='\0';
       BuscarItem(cadItem,&(datosItem[itemActual]));
       cadAux[itemActual][i]='\0';
       i = 0;
       datosItem[itemActual+1] = datosItem[itemActual];
       itemActual++;
       contItem = 0;
       cadItem[0] = '\0';
      }
     }
    }
    else{
     if (car!=10){
      if (car!=13){
       cadAux[itemActual][i] = car;
       i++;
      }
     }
    }
   }
   if (car==10){
    cadAux[itemActual][i]='\0';
    datosItem[0].laFoto.xIni = elArticulo.info[j].item.laFoto.xIni;
    datosItem[0].laFoto.yIni = elArticulo.info[j].item.laFoto.yIni;
    datosItem[0].laFoto.numFoto = elArticulo.info[j].item.laFoto.numFoto;
    elArticulo.info[j].item = datosItem[0];

    //Aqui pasamos los datos para justificar manualmente la primera cadena de la linea
    if (strlen(cadAux[0])>longitud_cadena) cadAux[0][longitud_cadena]='\0'; //recorta la cadena
    total = 0;
    if (itemActual>0){
     //Hay mas item en la primera linea
     for (k=0,total=0;k<=itemActual;k++)
      total = total + strlen(cadAux[k]);
     if (itemActual!=0){
      total = (longitud_cadena-total)/itemActual;
      resto = (longitud_cadena-total)%itemActual;
     }
     else{
      total = 0;
      resto = 0;
     }
     HacerJustificacion(cadAux[0],datosItem[0].letra,datosItem[0].justificacion,strlen(cadAux[0])+total);
    }
    else
      HacerJustificacion(cadAux[0],datosItem[0].letra,datosItem[0].justificacion,longitud_cadena);
   
    elArticulo.info[j].cadena = (char *)malloc(sizeof(char)*(strlen(cadAux[0])+1));
    strcpy(elArticulo.info[j].cadena,cadAux[0]);

    //Metemos el resto de items de una misma linea de texto
    if (itemActual>0){
     elArticulo.info[j].sig = (Ttexto*)malloc(sizeof(Ttexto));
     pAux = elArticulo.info[j].sig;
     for (k=1;k<=itemActual;k++){
      if (strlen(cadAux[k])>longitud_cadena) cadAux[k][longitud_cadena]='\0'; //recorta la cadena
      if (k==itemActual)
       HacerJustificacion(cadAux[k],datosItem[k].letra,datosItem[k].justificacion,strlen(cadAux[k])+total+resto);
      else
       HacerJustificacion(cadAux[k],datosItem[k].letra,datosItem[k].justificacion,strlen(cadAux[k])+total);
      pAux->cadena = (char *)malloc(sizeof(char)*(strlen(cadAux[k])+1));
      strcpy(pAux->cadena,cadAux[k]);
      pAux->sig = NULL;
      pAux->color = datosItem[k].color;
      pAux->letra = datosItem[k].letra;
      if (k<itemActual){
       pAux->sig = (struct Ttexto *)malloc(sizeof(Ttexto));
       pAux = (Ttexto *)pAux->sig;
       pAux->cadena = NULL;
       pAux->sig = NULL;
      }
     }
    }
    
    j++;
    elArticulo.numFilas++;
    i=0;
    datosItem[0] = datosItem[itemActual];
    itemActual = 0;
   }
   cont++;
  }


 //borro el contenido actual de letras
  rectfill(letras,0,0,639,479,bitmap_mask_color(letras));

  auxiliar = 80;
  for (j=0;j<30;j++)
  // if (elArticulo.texto[j]!=NULL) textout(letras, datafile[FUENTE_XM].dat,elArticulo.texto[j],1,(90+j*10),0xFFFFFF); //Soprta 80 caracteres y 30 filas
   if (elArticulo.info[j].cadena!=NULL){
    //auxiliar = 90+((j<<3)+(j<<1));
    auxiliar = auxiliar+10;
    switch (elArticulo.info[j].item.justificacion){
     case nojustificado: textout(letras, datafile[(elArticulo.info[j].item.letra)].dat,elArticulo.info[j].cadena,desX_cadena,auxiliar,elArticulo.info[j].item.color);     
                         if (elArticulo.info[j].sig!=NULL){
                          auxLongitud = strlen(elArticulo.info[j].cadena)*8;
                          pAux = elArticulo.info[j].sig;
                          while (pAux!=NULL){
                           textout(letras, datafile[(pAux->letra)].dat,pAux->cadena,desX_cadena+auxLongitud,auxiliar,pAux->color);
                           auxLongitud = auxLongitud+(strlen(pAux->cadena))*8;
                           pAux = (Ttexto *)pAux->sig;
                          }
                         }
                         break;
     case centro: textout_centre(letras, datafile[(elArticulo.info[j].item.letra)].dat,elArticulo.info[j].cadena,319,auxiliar,elArticulo.info[j].item.color); break;
     case izquierda: textout(letras, datafile[(elArticulo.info[j].item.letra)].dat,elArticulo.info[j].cadena,desX_cadena,auxiliar,elArticulo.info[j].item.color);
                     if (elArticulo.info[j].sig!=NULL){
                      auxLongitud = strlen(elArticulo.info[j].cadena)*8;
                      pAux = elArticulo.info[j].sig;
                      while (pAux!=NULL){
                       textout(letras, datafile[(pAux->letra)].dat,pAux->cadena,desX_cadena+auxLongitud,auxiliar,pAux->color);
                       auxLongitud = auxLongitud+(strlen(pAux->cadena))*8;
                       pAux = (Ttexto *)pAux->sig;
                      }
                     }
                     break;
                     //textout_justify(letras, datafile[indiceFuente].dat,elArticulo.info[j].cadena,JustificacionIzquierda(elArticulo.info[j].cadena,elArticulo.info[j].item.letra),639,(90+j*10),640,color); break;
     case derecha: textout(letras, datafile[(elArticulo.info[j].item.letra)].dat,elArticulo.info[j].cadena,(639-desX_cadena-JustificacionDerecha(elArticulo.info[j].cadena,elArticulo.info[j].item.letra)),auxiliar,elArticulo.info[j].item.color); break;//Soporta 80 caracteres y 30 filas
    }
    //textout(letras, datafile[(elArticulo.info[j].item.letra)].dat,elArticulo.info[j].cadena,0,(90+j*10),elArticulo.info[j].item.color);


    if ((elArticulo.info[j].item.laFoto.numFoto)!=0){
     if (!grises) select_palette(*(elArticulo.paletas[((elArticulo.info[j].item.laFoto.numFoto))]));
     blit(elArticulo.fotos[(elArticulo.info[j].item.laFoto.numFoto)],letras,0,elArticulo.info[j].item.laFoto.yIni,(((elArticulo.info[j].item.laFoto.xIni)<<3)+desX_cadena),auxiliar,elArticulo.fotos[(elArticulo.info[j].item.laFoto.numFoto)]->w,10);
    }

   }
  elArticulo.posFila = 29;

  if (!grises) select_palette(configuracion[PALETA_FONDO].dat);
  blit(fondo,videoTemporal,0,0,0,80,anchoX,321);
  //rectfill(videoTemporal,0,400,640,400,0);
  MostrarBotones();
//  MostrarArticulosPantalla(&losArticulos,letras);
  masked_blit(letras,videoTemporal,0,0,0,0,anchoX,altoY);
  if (!grises) select_palette(configuracion[PALETA_LOGO].dat);
  blit(configuracion[FOTO_LOGO].dat,videoTemporal,0,0,0,0,anchoX,80);
  MostrarTitulo(letras);
  MostrarInfoArticulo(letras);
  if (raton) show_mouse(NULL);
   blit(videoTemporal,screen,0,0,0,0,anchoX,altoY);
  if (raton) show_mouse(screen);
 }
 else{
  allegro_exit();
  allegro_message("Error en articulo");
  exit(-1);
 }
}

/************************************************/
void InicializarArticulo(){
 int i;
 for (i=0;i<maxInfo;i++){
//  elArticulo.texto[i]=NULL;
  elArticulo.info[i].cadena=NULL;
  elArticulo.info[i].item.color=color_blanco;
  elArticulo.info[i].item.letra=FUENTE_PC;
  elArticulo.info[i].item.justificacion=centro;
  elArticulo.info[i].item.laFoto.numFoto = 0; //Al ser 0, no hay foto
  elArticulo.info[i].sig = NULL;
 }
 elArticulo.numFotos = 0;     //Pone 0 fotos en al articulo
 for (i=0;i<maxFotos;i++)
  elArticulo.fotos[i] = NULL;
}

/************************************************/
void EliminarArticulo(){
 int i;
 Ttexto * aux1, *aux2;
 for (i=0;i<maxInfo;i++){
  free(elArticulo.info[i].cadena);
  elArticulo.info[i].cadena = NULL;
  aux1 = elArticulo.info[i].sig;
  elArticulo.info[i].sig = NULL;
  while (aux1 != NULL){
   aux2 = aux1;
   free(aux2->cadena);
   aux2->cadena = NULL;
   aux1 = (Ttexto *)aux1->sig;
   free (aux2);
  }
 }
 elArticulo.numFotos = 0; 
}


/************************************************/
void LeerArticulos(TArticulos *articulos, DATAFILE *revista){
 int aux,i,total,cont=11;
 int numArticulos, indiceArticulo;
 char car;
 char *endp;
 char cadAux[500];
 total = configuracion[TEXTO_ARTICULOS].size;
 cadAux[0]='\0';
 i=0;
 car=((char *)configuracion[TEXTO_ARTICULOS].dat)[cont];;
 while ((car!=']')&&(car!=10)){
  if (car!=13){
   cadAux[i]=car;
   i++;
  }
  cont++;
  car=((char *)configuracion[TEXTO_ARTICULOS].dat)[cont];
 }
 cadAux[i]='\0';
 numArticulos = strtod(cadAux,&endp);
 articulos->numArticulos = numArticulos;
 
 cadAux[0]='\0';
 car=((char *)configuracion[TEXTO_ARTICULOS].dat)[cont];
 while (car!=10){
  cont++;
  car=((char *)configuracion[TEXTO_ARTICULOS].dat)[cont];
 }
 cont++;

 
 for (i=1;i<=numArticulos;i++){//Pongo todos los articulos
  car=0;
  aux=0;
  while (car!=' '){
   car = ((char *)configuracion[TEXTO_ARTICULOS].dat)[cont];
   if ((car!=' ')&&(car!=10)&&(car!=13)){
    cadAux[aux]=car;
    aux++;
   }
   cont++;
  }
  cadAux[aux]='\0';
  //indiceArticulo=(int)strtod(cadAux,&endp); //Cojo el indice del texto
  //Busca el objeto en la revista
  articulos->articulo[i-1] = find_datafile_object(revista,cadAux);
  
  //articulos->articulo[i-1] = indiceArticulo;
  cadAux[0]='\0';

  car=0;
  aux=0;  
  while (car!=10){
   car = ((char *)configuracion[TEXTO_ARTICULOS].dat)[cont];
   if ((car!=10)&&(car!=13)){
    cadAux[aux]=car;
    aux++;
   }
   cont++;
  }
  cadAux[aux]='\0';
  articulos->textoArticulo[i-1] = malloc((sizeof(char)*aux)+1);
  strcpy(articulos->textoArticulo[i-1],cadAux);
  cadAux[0]='\0';

  //Aqui cogemos el nombre del autor
  car=0;
  aux=0;  
  while (car!=10){
   car = ((char *)configuracion[TEXTO_ARTICULOS].dat)[cont];
   if ((car!=10)&&(car!=13)){
    cadAux[aux]=car;
    aux++;
   }
   cont++;
  }
  cadAux[aux]='\0';
  articulos->textoPlataForma[i-1] = malloc((sizeof(char)*aux)+1);
  strcpy(articulos->textoPlataForma[i-1],cadAux);
  cadAux[0]='\0';

  car=0;
  aux=0;  
  while (car!=10){
   car = ((char *)configuracion[TEXTO_ARTICULOS].dat)[cont];
   if ((car!=10)&&(car!=13)){
    cadAux[aux]=car;
    aux++;
   }
   cont++;
  }
  cadAux[aux]='\0';
  articulos->textoAutor[i-1] = malloc((sizeof(char)*aux)+1);
  strcpy(articulos->textoAutor[i-1],cadAux);
  cadAux[0]='\0';

  car=0;
  aux=0;  
  while (car!=10){
   car = ((char *)configuracion[TEXTO_ARTICULOS].dat)[cont];
   if ((car!=10)&&(car!=13)){
    cadAux[aux]=car;
    aux++;
   }
   cont++;
  }
  cadAux[aux]='\0';
  articulos->textoWeb[i-1] = malloc((sizeof(char)*aux)+1);
  strcpy(articulos->textoWeb[i-1],cadAux);
  cadAux[0]='\0';
  
  car=0;
  aux=0;  
  while (car!=10){
   car = ((char *)configuracion[TEXTO_ARTICULOS].dat)[cont];
   if ((car!=10)&&(car!=13)){
    cadAux[aux]=car;
    aux++;
   }
   cont++;
  }
  cadAux[aux]='\0';
  articulos->textoCorreo[i-1] = malloc((sizeof(char)*aux)+1);
  strcpy(articulos->textoCorreo[i-1],cadAux);
  cadAux[0]='\0';  
 }
}

/************************************************/
void MostrarArticulosPantalla(TArticulos *losArticulos, BITMAP *pantalla){
 int i,numeros,articuloActual;
 int blanco = color_blanco;
 int amarillo = color_amarillo;
 int aux;
 numeros = losArticulos->numArticulos;
 articuloActual = losArticulos->articuloActual;
 textout_centre(pantalla, datafile[FUENTE_PC].dat,(losArticulos->textoArticulo[articuloActual]),319,420,blanco); //Soporta 80 caracteres y 30 filas
 for (i=1,aux=10;i<4;i++,aux=aux+10)
  if (i+articuloActual<numeros) textout_centre(pantalla, datafile[FUENTE_XM].dat,(losArticulos->textoArticulo[i+articuloActual]),319,420+aux,amarillo); //Soporta 80 caracteres y 30 filas

}

/************************************************/
void ScrollArribaArticulos(TArticulos *losArticulos, BITMAP *pantalla){
 if (losArticulos->articuloActual>0){
  losArticulos->articuloActual-=1;
  //rectfill(pantalla,120,400,520,470,bitmap_mask_color(pantalla));
  rectfill(pantalla,187,420,440,459,bitmap_mask_color(pantalla));
  MostrarArticulosPantalla(losArticulos,pantalla);
  //if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
  //blit(configuracion[FOTO_BARRA].dat,videoTemporal,120,0,120,400,400,70);
  //masked_blit(pantalla,videoTemporal,120,400,120,400,400,70);
  rectfill(videoTemporal,187,420,440,459,color_negro);
  masked_blit(pantalla,videoTemporal,187,420,187,420,253,39);
 // if ((raton)&&(mouse_x>=187)&&(mouse_x<=440)&&(mouse_y>=420)&&(mouse_y<=459)){
  if (raton){
   show_mouse(NULL);
   //blit(videoTemporal,screen,120,400,120,400,400,70);
   blit(videoTemporal,screen,187,420,187,420,253,39);
   show_mouse(screen);
  }
  else{
   //blit(videoTemporal,screen,120,400,120,400,400,70);
   blit(videoTemporal,screen,187,420,187,420,253,39);
  }
 }
}

/************************************************/
void ScrollAbajoArticulos(TArticulos *losArticulos, BITMAP *pantalla){
 if (losArticulos->articuloActual<(losArticulos->numArticulos-1)){
  losArticulos->articuloActual+=1;
  //rectfill(pantalla,120,400,520,470,bitmap_mask_color(pantalla));
  rectfill(pantalla,187,420,440,459,bitmap_mask_color(pantalla));
  MostrarArticulosPantalla(losArticulos,pantalla);
  //if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
  //blit(configuracion[FOTO_BARRA].dat,videoTemporal,120,0,120,400,400,70);
  rectfill(videoTemporal,187,420,440,459,color_negro);
  //masked_blit(pantalla,videoTemporal,120,400,120,400,400,70);
  masked_blit(pantalla,videoTemporal,187,420,187,420,253,39);
  //if ((raton)&&(mouse_x>=187)&&(mouse_x<=440)&&(mouse_y>=420)&&(mouse_y<=459)){
  if (raton){
   show_mouse(NULL);
   //blit(videoTemporal,screen,120,400,120,400,400,70);
   blit(videoTemporal,screen,187,420,187,420,253,39);
   show_mouse(screen);
  }
  else{
   //blit(videoTemporal,screen,120,400,120,400,400,70);
   blit(videoTemporal,screen,187,420,187,420,253,39);
  }
 }
}

/************************************************/
void InicializarLosArticulos(TArticulos *losArticulos){
 unsigned char i;
 losArticulos->numArticulos = 0;
 losArticulos->articuloActual = 0;
 for (i=0;i<maxArticulos;i++){
  losArticulos->articulo[i]=0;
  losArticulos->textoArticulo[i]=NULL;
  losArticulos->textoPlataForma[i]=NULL;
  losArticulos->textoAutor[i]=NULL;
  losArticulos->textoWeb[i]=NULL;
  losArticulos->textoCorreo[i]=NULL;
 }
}

/************************************************/
void EliminarLosArticulos(TArticulos* losArticulos){
 losArticulos->numArticulos = 0;
 losArticulos->articuloActual = 0;
 for (i=0;i<maxArticulos;i++){
  losArticulos->articulo[i]=0;
  free(losArticulos->textoArticulo[i]); //Libero la memoria ocupada
  free(losArticulos->textoPlataForma[i]);
  free(losArticulos->textoAutor[i]);
  free(losArticulos->textoWeb[i]);
  free(losArticulos->textoCorreo[i]);
  losArticulos->textoArticulo[i]=NULL;  //Por si acaso no la libera
  losArticulos->textoPlataForma[i]=NULL;
  losArticulos->textoAutor[i]=NULL;
  losArticulos->textoWeb[i]=NULL;
  losArticulos->textoCorreo[i]=NULL;
 }
}

/************************************************/
void ControlaEventos(){
 boolean salir=FALSE;
 int ratonX=0,ratonY=0;    //Para los mickeys del raton
 int ratonZantes;
 ratonZantes = mouse_z;
 while (!salir){
  ControlaEventoTeclado(&salir);
  ControlaEventoRaton(&ratonX, &ratonY, &ratonZantes, &salir);
  ControlaEventoJoystick();
 }
}

/************************************************/
void MostrarTitulo(BITMAP *pantalla){
 //Muestra el titulo del articulo en la zona superior izquierda
 int azulado = makecol(128,255,255);
  rectfill(pantalla,10,60,473,80,bitmap_mask_color(pantalla));

  textout(pantalla, datafile[FUENTE_XM].dat,"Seccion",10,30,azulado); //Soporta 80 caracteres y 30 filas
  textout(pantalla, datafile[FUENTE_XM].dat,losArticulos.textoPlataForma[(losArticulos.articuloActual)],74,30,color_blanco); //Soporta 80 caracteres y 30 filas

  textout(pantalla, datafile[FUENTE_XM].dat,"Titulo",10,40,azulado); //Soporta 80 caracteres y 30 filas
  textout(pantalla, datafile[FUENTE_XM].dat,losArticulos.textoArticulo[(losArticulos.articuloActual)],74,40,color_blanco); //Soporta 80 caracteres y 30 filas
  
  textout(pantalla, datafile[FUENTE_XM].dat,"Autor",10,50,azulado); //Soporta 80 caracteres y 30 filas
  textout(pantalla, datafile[FUENTE_XM].dat,losArticulos.textoAutor[(losArticulos.articuloActual)],74,50,color_blanco); //Soporta 80 caracteres y 30 filas

  textout(pantalla, datafile[FUENTE_XM].dat,"Correo",10,60,azulado); //Soporta 80 caracteres y 30 filas
  textout(pantalla, datafile[FUENTE_XM].dat,losArticulos.textoCorreo[(losArticulos.articuloActual)],74,60,color_blanco); //Soporta 80 caracteres y 30 filas
  
  textout(pantalla, datafile[FUENTE_XM].dat,"Web",10,70,azulado); //Soporta 80 caracteres y 30 filas
  textout(pantalla, datafile[FUENTE_XM].dat,losArticulos.textoWeb[(losArticulos.articuloActual)],74,70,color_blanco); //Soporta 80 caracteres y 30 filas
  
  if (!grises) select_palette(configuracion[PALETA_LOGO].dat);
  blit(configuracion[FOTO_LOGO].dat,videoTemporal,10,30,10,30,474,50);
  masked_blit(pantalla,videoTemporal,10,30,10,30,474,50);
}

/************************************************/
void MostrarInfoArticulo(BITMAP *pantalla){
//Muestra las p ginas totales del articulo y la situaci¢n en el mismo
  int filas,filaActual;
//  char cadena[33];
//  char temp[128];
  filas = elArticulo.numFilas;
  filaActual = elArticulo.posFila;
  filaActual /= 30;
  filas /=30;
  if (((filas % 30)!=0)||(filas<30)) filas+=1;
  if (((filaActual % 30)!=0)||(filaActual<30)) filaActual+=1;
  if (filaActual>filas) filaActual=filas;
  //cadena[0]='\0';
  //itoa(filaActual,cadena,10);
  //strcpy(temp,cadena);
  //strcat(temp,"/");
  //cadena[0]='\0';
  //itoa(filas,cadena,10);
  //strcat(temp,cadena);
  //strcat(temp," pag");
  rectfill(pantalla,540,45,639,75,bitmap_mask_color(pantalla));
  //textout(pantalla, datafile[FUENTE_GRANDE].dat,temp,540,45,makecol(255,255,255)); //Soporta 80 caracteres y 30 filas
  textprintf(pantalla, datafile[FUENTE_GRANDE].dat,540,45,color_blanco,"%d/%d pag",filaActual,filas); //Soporta 80 caracteres y 30 filas
  if (!grises) select_palette(configuracion[PALETA_LOGO].dat);
  blit(configuracion[FOTO_LOGO].dat,videoTemporal,540,45,540,45,100,30);
  masked_blit(pantalla,videoTemporal,540,45,540,45,100,30);
//  blit(videoTemporal,screen,540,50,540,50,100,30); Para volcar a video se encarga otro
}

/************************************************/
void MostrarBotones(){
 int i;
 if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
 blit(configuracion[FOTO_BARRA].dat,videoTemporal,0,0,0,400,640,80);
 if (sonido==FALSE){
  for (i=0;i<3;i++){
   line(videoTemporal,122+i,449,136+i,467,makecol(255,0,0));
   line(videoTemporal,122+i,467,136+i,449,makecol(255,0,0));
   //line(videoTemporal,16+i,422,56+i,460,makecol(255,0,0));
   //line(videoTemporal,16+i,460,56+i,422,makecol(255,0,0));
  }
 }
}

/************************************************/
int JustificacionDerecha(char *cadena,int tipoLetra){
//Saca los pixels X que debe de poner en testout para justificar
 int longitud=strlen(cadena);
 int i=longitud;
 if (longitud>0){
  while ((cadena[i]==' ')&&(i>=0))
   i--;
 };
 switch (tipoLetra){
  case FUENTE_GRANDE:  i=(i<<3)+(i<<1); break; //i=i*10
  default:  i = (i<<3); //i=i*8;
 } 
 return (i);
}


/************************************************/
void ScrollArriba30Lineas(){
 //Si pulsamod KEY_PUAGDOWN
 int auxiliar; 
 int i,j=0,total=0;
 short int auxLongitud;
 Ttexto * pAux;

 if (elArticulo.posFila<elArticulo.numFilas){
/*  if ((elArticulo.posFila+30)>(elArticulo.numFilas))
   total= elArticulo.numFilas - elArticulo.posFila;
  else*/
   total=30;
  if (!grises) select_palette(configuracion[PALETA_FONDO].dat);
  blit(fondo,videoTemporal,0,0,0,80,640,320);
  //rectfill(videoTemporal,0,400,640,400,0);
  rectfill(letras,0,90,639,400,bitmap_mask_color(letras));


  auxiliar = 80;
  for (i=(elArticulo.posFila+1);i<(elArticulo.posFila+total+1);i++,j++){
   if (elArticulo.info[i].cadena!=NULL){
    //auxiliar = 90+(j<<3)+(j<<1);
    auxiliar = auxiliar+10;
    switch (elArticulo.info[i].item.justificacion){
     case nojustificado: textout(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,desX_cadena,auxiliar,elArticulo.info[i].item.color); break;
                         //textout(letras, datafile[indiceFuente].dat,elArticulo.info[i].cadena,0,(90+j*10),color); break;
     case centro: textout_centre(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,319,auxiliar,elArticulo.info[i].item.color); break;
     case izquierda: textout(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,desX_cadena,auxiliar,elArticulo.info[i].item.color); break;
                     //textout_justify(letras, datafile[indiceFuente].dat,elArticulo.info[i].cadena,JustificacionIzquierda(elArticulo.info[i].cadena,elArticulo.info[i].item.letra),639,(90+j*10),640,color); break;
     case derecha: textout(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,(639-desX_cadena-JustificacionDerecha(elArticulo.info[i].cadena,elArticulo.info[i].item.letra)),auxiliar,elArticulo.info[i].item.color); break;//Soporta 80 caracteres y 30 filas
    }
    //textout(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,0,(90+j*10),elArticulo.info[i].item.color);

    //Pongo el resto de los items de la misma linea de texto
    if (elArticulo.info[i].sig!=NULL){
     auxLongitud = (strlen(elArticulo.info[i].cadena)*8);
     pAux = elArticulo.info[i].sig;
     while (pAux!=NULL){
      textout(letras, datafile[(pAux->letra)].dat,pAux->cadena,desX_cadena+auxLongitud,auxiliar,pAux->color);
      auxLongitud = auxLongitud+(strlen(pAux->cadena)*8);
      pAux = (Ttexto *)pAux->sig;
     }
    }



    if ((elArticulo.info[i].item.laFoto.numFoto)!=0){
      if (!grises) select_palette(*(elArticulo.paletas[((elArticulo.info[i].item.laFoto.numFoto))]));
      blit(elArticulo.fotos[(elArticulo.info[i].item.laFoto.numFoto)],letras,0,elArticulo.info[i].item.laFoto.yIni,(((elArticulo.info[i].item.laFoto.xIni)<<3)+desX_cadena),auxiliar,elArticulo.fotos[(elArticulo.info[i].item.laFoto.numFoto)]->w,10);
    }

    
   }
  }
  elArticulo.posFila+=total;
  masked_blit(letras,videoTemporal,0,0,0,0,640,480);
  MostrarInfoArticulo(letras);
  //if ((raton)&&(mouse_y>=30)&&(mouse_y<400)){
  if (raton){
   show_mouse(NULL);
   blit(videoTemporal,screen,0,50,0,50,640,350);
   show_mouse(screen);
  }
  else blit(videoTemporal,screen,0,50,0,50,640,350);
 }
}

/************************************************/
void ScrollAbajo30Lineas(){
 //Si pulsamos KEY_PUAGUP
 int auxiliar;
 int i,j=0,total=0;
 short int auxLongitud;
 Ttexto * pAux;

 if (elArticulo.posFila>29){
  total=30;
  if (elArticulo.posFila<60)
   elArticulo.posFila = 0;
  else
   elArticulo.posFila = elArticulo.posFila-59;
   
  if (!grises) select_palette(configuracion[PALETA_FONDO].dat);
  blit(fondo,videoTemporal,0,0,0,80,640,320);
  //rectfill(videoTemporal,0,400,640,400,0);
  rectfill(letras,0,90,639,400,bitmap_mask_color(letras));


  auxiliar = 80;
  for (i=(elArticulo.posFila);i<(elArticulo.posFila+total);i++,j++){
   if (elArticulo.info[i].cadena!=NULL){
    //auxiliar = 90+(j<<3)+(j<<1);
    auxiliar = auxiliar+10;
    switch (elArticulo.info[i].item.justificacion){
     case nojustificado: textout(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,desX_cadena,auxiliar,elArticulo.info[i].item.color); break;
                         //textout(letras, datafile[indiceFuente].dat,elArticulo.info[i].cadena,0,(90+j*10),color); break;
     case centro: textout_centre(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,319,auxiliar,elArticulo.info[i].item.color); break;
     case izquierda: textout(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,desX_cadena,auxiliar,elArticulo.info[i].item.color); break;
                     //textout_justify(letras, datafile[indiceFuente].dat,elArticulo.info[i].cadena,JustificacionIzquierda(elArticulo.info[i].cadena,elArticulo.info[i].item.letra),639,(90+j*10),640,color); break;
     case derecha: textout(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,(639-desX_cadena-JustificacionDerecha(elArticulo.info[i].cadena,elArticulo.info[i].item.letra)),auxiliar,elArticulo.info[i].item.color); break;//Soporta 80 caracteres y 30 filas
    }
    //textout(letras, datafile[(elArticulo.info[i].item.letra)].dat,elArticulo.info[i].cadena,0,(90+j*10),elArticulo.info[i].item.color);

    //Pongo el resto de los items de la misma linea de texto
    if (elArticulo.info[i].sig!=NULL){
     auxLongitud = (strlen(elArticulo.info[i].cadena)*8);
     pAux = elArticulo.info[i].sig;
     while (pAux!=NULL){
      textout(letras, datafile[(pAux->letra)].dat,pAux->cadena,desX_cadena+auxLongitud,auxiliar,pAux->color);
      auxLongitud = auxLongitud+(strlen(pAux->cadena)*8);
      pAux = (Ttexto *)pAux->sig;
     }
    }


    if ((elArticulo.info[i].item.laFoto.numFoto)!=0){
      if (!grises) select_palette(*(elArticulo.paletas[((elArticulo.info[i].item.laFoto.numFoto))]));
      blit(elArticulo.fotos[(elArticulo.info[i].item.laFoto.numFoto)],letras,0,elArticulo.info[i].item.laFoto.yIni,(((elArticulo.info[i].item.laFoto.xIni)<<3)+desX_cadena),auxiliar,elArticulo.fotos[(elArticulo.info[i].item.laFoto.numFoto)]->w,10);
    }

    
   }
  }
  elArticulo.posFila=elArticulo.posFila+29;
  masked_blit(letras,videoTemporal,0,0,0,0,640,480);
  MostrarInfoArticulo(letras);
  //if ((raton)&&(mouse_y>=30)&&(mouse_y<400)){
  if (raton){
   show_mouse(NULL);
   blit(videoTemporal,screen,0,50,0,50,640,350);
   show_mouse(screen);
  }
  else blit(videoTemporal,screen,0,50,0,50,640,350);
 }
}

/************************************************/
void AccionSonido(){
 int i; 
 if (driverSonido!=FALSE){
  if (sonido!=FALSE){
   sonido = FALSE;
   if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
   //blit(configuracion[FOTO_BARRA].dat,videoTemporal,0,0,0,400,80,80);
   blit(configuracion[FOTO_BARRA].dat,videoTemporal,120,40,120,440,30,30);
   for (i=0;i<3;i++){
    //line(videoTemporal,16+i,422,56+i,460,makecol(255,0,0));
    //line(videoTemporal,16+i,460,56+i,422,makecol(255,0,0));
    line(videoTemporal,122+i,449,136+i,467,color_rojo);
    line(videoTemporal,122+i,467,136+i,449,color_rojo);
   }
   //pause_mod(); //Para el MOD
   midi_pause();
  }
  else{
   sonido = TRUE;
   if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
   //blit(configuracion[FOTO_BARRA].dat,videoTemporal,0,0,0,400,80,80);
   blit(configuracion[FOTO_BARRA].dat,videoTemporal,120,40,120,440,30,30);
   //resume_mod();
   midi_resume();
  }
  //if ((mouse_y>=380)&&(mouse_y<480)&&(mouse_x>=0)&&(mouse_x<80)){
  if (raton){
   show_mouse(NULL);
    //blit(videoTemporal,screen,0,400,0,400,80,80);
    blit(videoTemporal,screen,120,440,120,440,30,30);
   show_mouse(screen);
  }
  else{
   //blit(videoTemporal,screen,0,400,0,400,80,80);
   blit(videoTemporal,screen,120,440,120,440,30,30);
  }
 }
}

/************************************************/
void SiguienteCancion(){
 if (driverSonido!=FALSE){
  stop_midi();
  if (numMidiActual>5) numMidiActual=0;
  else
   if (numMidiActual<0) numMidiActual=5;
  switch (numMidiActual){
   case 0: archivoMidi = configuracion[MIDI_1].dat; break;
   case 1: archivoMidi = configuracion[MIDI_2].dat; break;
   case 2: archivoMidi = configuracion[MIDI_3].dat; break;
   case 3: archivoMidi = configuracion[MIDI_4].dat; break;
   case 4: archivoMidi = configuracion[MIDI_5].dat; break;
   case 5: archivoMidi = configuracion[MIDI_6].dat; break;
  }
  play_midi(archivoMidi,TRUE);
 }
}

/************************************************/
void HacerJustificacion(char *cadena,int tipoLetra,enum TitemJustificacion justificacion, short int total){
 char cadAux[500];
 unsigned char listaEspacios[500];
 int i=0;
 int j=0;
 int espacios=0;  //contiene el n1 de espacios en la cadena original
 int espaciosIzquierda=0; //Los espacios a la izquierda de la cadena
 int espacioLibre;        //El espacio libre que queda a la derecha
 int bucleEspacios=0;     //El n§ de espacios a poner en cada espacio original
 int resto=0;
 int cont=0;
 int longitud=strlen(cadena);
 int salir;
 int indiceEspacio;
 int topeCadena=0;
 
 strcpy(cadAux,cadena);
 if (longitud>0){
  //Salto todos los espacios en blanco de la izquierda
  while ((cadAux[i]==' ')&&(i<longitud))
   i++;
  espaciosIzquierda = i;
   //Cuento espacios en blanco
  while(i<longitud){
   if (cadAux[i]==' ') espacios++;
   i++;
  }

  if (justificacion==izquierda){
   switch (tipoLetra){
    case FUENTE_GRANDE:   topeCadena= total-15; break;
    default: topeCadena = total;
   }

   espacioLibre = topeCadena-longitud;
   if (espacios==0){
    bucleEspacios = 0;
    resto = 0;
   }
   else{
    bucleEspacios = espacioLibre/espacios;
    resto = espacioLibre % espacios;
   }

   cont=espaciosIzquierda;
   indiceEspacio=0;
   if (espacioLibre<espacios){
    for (i=espaciosIzquierda;i<longitud;i++,cont++){
     if ((cadAux[i]==' ')&&(indiceEspacio<resto)){
      cadena[cont]=' ';
      cadena[cont+1]=' ';
      cont++;
      indiceEspacio++;
     }
     else cadena[cont]=cadAux[i];
    }
    cadena[cont]='\0';
   }
   else{
    indiceEspacio=0;
    for (i=0;i<espacios;i++) listaEspacios[i]=bucleEspacios+1;
    for (i=0;i<resto;i++) listaEspacios[i]++;
    for (i=espaciosIzquierda;i<longitud;i++,cont++){
     if (cadAux[i]==' '){
      for (j=0;j<listaEspacios[indiceEspacio];j++,cont++)
       cadena[cont]=' ';
      indiceEspacio++;
      cont--;
     }
     else cadena[cont]=cadAux[i];
    }
    cadena[cont]='\0';
   }
  }
 }
}

/************************************************/
void GeneraColores(){
  color_rojo = makecol(255,0,0); color_verde = makecol(0,180,0); //Para optimizar
  color_azul = makecol(0,0,255); color_negro = makecol(0,0,0);
  color_blanco = makecol(255,255,255);  color_marron = makecol(178,139,4);
  color_amarillo = makecol(252,255,17); color_violeta = makecol(211,128,246);
  color_verdeFosforito = makecol(80,255,66); color_azulClaro = makecol(29,226,255);
  color_naranja = makecol(255,125,29);       color_gris = makecol(136,148,152);
  color_grisClaro = makecol(222,230,231);
}

/************************************************/
void inc_crono(){
 crono++;
}
END_OF_FUNCTION(inc_crono);

/************************************************/
void Presentacion(DATAFILE *datos){
//Muestra la foto de portada y hace efecto FADER
 BITMAP *foto;
 BITMAP *issue;
 BITMAP *exilium;
 int posX,posY;
 foto = datos[FOTO_PRESENTACION].dat;
 issue = datos[FOTO_ISSUE].dat;
 exilium = datos[FOTO_EXILIUM].dat;
 posX = (640-(foto->w))>>1;
 posY = (480-(foto->h))>>1;
 set_color_depth(8);
 if (set_gfx_mode(GFX_AUTODETECT,640,480,0,0)!=0){
  allegro_message("Error en modo de video\nCambie de driver grafico en CFG");
  allegro_exit();
  printf("Error en modo de video\nCambie de driver grafico en CFG");
  exit(-1);
 }
 set_palette(black_palette);

 
 blit(foto,screen,0,0,posX,posY,foto->w,foto->h);
 ActivaMOD(); //Activamos la musica
 crono = 0;
 LOCK_VARIABLE(crono);
 LOCK_FUNCTION(inc_crono);
 install_int(inc_crono,500); //Arranco el temporizador de segundos
 

 Desfundir (datos[PALETA_PRESENTACION].dat);
 //fade_in(datos[PALETA_PRESENTACION].dat,1);

 while ((crono<27)&&!keypressed());
 //Mostrar Issue a los 13.5 segundos
 masked_blit(issue,screen,0,0,505,340,issue->w,issue->h);
 

 while ((crono<33)&&!keypressed());
 //Mostrar Diskmag Exsilium (Destierro) a los 16.5 segundos
 masked_blit(exilium,screen,0,0,25,30,exilium->w,exilium->h);
 

 while ((crono<46)&&!keypressed()); //Espera a fundir a los 23 segundos
 //fade_out(1);
 Fundir();

 while ((crono<51)&&!keypressed()); //Espera a los 25.5 segundos
 
 
 remove_int(inc_crono);
 clear_keybuf();
}

/************************************************/
int ActivaMOD(){
 char car, aux[255], archivoSonido[255];
 char *endp;
 int i,cont,tope,numCanales=0;


 reserve_voices (numCanales, -1);     // call this before install_sound if needed
 if (install_sound (DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) < 0){
   //printf ("Error initializing sound card");
   return -1;
 }
 
 numMidiActual = 0;
 archivoMidi = configuracion[MIDI_1].dat;
 set_volume(numVolumenActual,numVolumenActual);
 play_midi(archivoMidi,TRUE);
/*    if (install_mod (numCanales) < 0)    // call install_mod only after install_sound
        {
        //printf ("Error setting digi voices");
        return -1;
        }

    archivoMOD = load_mod (archivoSonido);
    if (archivoMOD == NULL)
        {
        //printf ("Error reading introgho.mod");
        return -1;
        }

    play_mod (archivoMOD, TRUE);*/
    
 sonido = driverSonido = TRUE;
}


/************************************************/
void DestruyeMOD(){
// stop_mod();
// destroy_mod (archivoMOD);
  midi_pause();
//  destroy_midi(archivoMidi);
}

/************************************************/
void Fundir(){
  //Soluciona el problema del retrazo en VOODOO
 PALETTE paleta;
 boolean salir = FALSE; 
 short int i = 0;
 unsigned char j; 
 get_palette(paleta);
 while (!salir){
  salir = TRUE;
  for (i=0;i<256;i++){
   if (paleta[i].r!=0){
     salir = FALSE;
     paleta[i].r -=1;
   }
   if (paleta[i].g!=0){
     salir = FALSE;
     paleta[i].g -=1;
   }
   if (paleta[i].b!=0){
     salir = FALSE;
     paleta[i].b -=1;
   }
  }

  for (j=0;j<2;j++)
   vsync();
  set_palette(paleta);
 }
}

/************************************************/
void Desfundir(PALETTE paleta){
//Desfunde la imagen bien en VODOO
 PALETTE pal;
 boolean salir = FALSE;
 unsigned char j;
 short int i = 0;
 
 get_palette(pal);
 while (!salir){
  salir = TRUE;
  for (i=0;i<256;i++){
   if (pal[i].r!=paleta[i].r){
     salir = FALSE;
     pal[i].r +=1;
   }
   if (pal[i].g!=paleta[i].g){
     salir = FALSE;
     pal[i].g +=1;
   }
   if (pal[i].b!=paleta[i].b){
     salir = FALSE;
     pal[i].b +=1;
   }
  }

  for (j=0;j<4;j++)
   vsync();
  set_palette(pal);
 }
}

/************************************************/
void Despedida(DATAFILE *datos){
 BITMAP *foto;
 BITMAP *issue;
 BITMAP *exilium;
 short int i;
 int posX,posY;
 foto = datos[FOTO_PRESENTACION].dat;
 issue = datos[FOTO_ISSUE].dat;
 exilium = datos[FOTO_EXILIUM].dat;
 posX = (640-(foto->w))>>1;
 posY = (480-(foto->h))>>1;
 set_color_depth(8);
 if (set_gfx_mode(GFX_AUTODETECT,640,480,0,0)!=0){
  allegro_message("Error en modo de video\nCambie de driver grafico CFG");
  allegro_exit();
  printf("Error en modo de video\nCambie de driver grafico CFG");
  exit(-1);
 }
 set_palette(black_palette);

 
 blit(foto,screen,0,0,posX,posY,foto->w,foto->h);
 masked_blit(issue,screen,0,0,505,340,issue->w,issue->h);
 masked_blit(exilium,screen,0,0,25,30,exilium->w,exilium->h); 
 crono=0;
 LOCK_VARIABLE(crono);
 LOCK_FUNCTION(inc_crono);
 install_int(inc_crono,500); //Arranco el temporizador de segundos

 Desfundir (datos[PALETA_PRESENTACION].dat);
 //fade_in(datos[PALETA_PRESENTACION].dat,1);

 
 while (crono<9);

 Fundir(); 
 
 remove_int(inc_crono);
}

/************************************************/
void ReguladorVolumen(unsigned char tipo){
 short int volumen,i;
 boolean salir=FALSE;
 BITMAP *captura;
 captura = create_bitmap(640,50);
// volumen = get_mod_volume();
 volumen = numVolumenActual;
 //if ((raton)&&(mouse_y>=300)&&(mouse_y<370)){
 if (raton){
   show_mouse(NULL);
   blit(screen,captura,0,320,0,0,640,50);
   show_mouse(screen);   
 }
 else blit(screen,captura,0,320,0,0,640,50);

 switch(tipo){
  case mas: while (key[KEY_PLUS_PAD]){
             if (volumen<251) volumen +=5;
             //if ((raton)&&(mouse_y>=300)&&(mouse_y<370)){
             if (raton){
               show_mouse(NULL);
               vsync();
               triangle(screen,5,365,45,325,45,365,color_verdeFosforito);
               rectfill(screen,70,325,(volumen<<1)+70,365,color_verdeFosforito);
               show_mouse(screen);
             }
             else{
              vsync();
              triangle(screen,5,365,45,325,45,365,color_verdeFosforito);
              rectfill(screen,70,325,(volumen<<1)+70,365,color_verdeFosforito);
             }
//             set_mod_volume(volumen);
             set_volume(volumen,volumen);
            }
            break;
  
  case menos: while (key[KEY_MINUS_PAD]){
               if (volumen>5) volumen-=5;
               //if ((raton)&&(mouse_y>=300)&&(mouse_y<370)){
               if (raton){
                show_mouse(NULL);
                vsync();
                blit(captura,screen,0,0,0,320,640,50);
                triangle(screen,5,365,45,325,45,365,color_verdeFosforito);
                rectfill(screen,70,325,(volumen<<1)+70,365,color_verdeFosforito);
                show_mouse(screen);
               }
               else{
                vsync();
                blit(captura,screen,0,0,0,320,640,50);
                triangle(screen,5,365,45,325,45,365,color_verdeFosforito);
                rectfill(screen,70,325,(volumen<<1)+70,365,color_verdeFosforito);
               }
//               set_mod_volume(volumen);
               set_volume(volumen,volumen);
              }
              break;
 }


 numVolumenActual = volumen;
 //if ((raton)&&(mouse_y>=300)&&(mouse_y<370)){
 if (raton){
  show_mouse(NULL);
  blit(captura,screen,0,0,0,320,640,50);
  show_mouse(screen);
 }
 else blit(captura,screen,0,0,0,320,640,50);

 destroy_bitmap(captura);

 
// vsync();
}

/************************************************/
void MostrarLineaComandos(void){
 printf("\n Diskmag Exilium issue #2   Grupo: SLIDERS   Fecha: 26-11-2002\n");
 printf("==============================================================\n\n");
 printf(" exilium [8|16] [no] [/?|?] [setup]\n\n");
 printf("   8    --> 8 bits de color\n");
 printf("  16    --> 16 bits de color\n");
 printf("  no    --> No visualiza la presentacion\n");
 printf("  setup --> Muestra el menu de configuracion\n");
 printf("  /?    --> Muestra ayuda\n\n");
 printf("Ejemplo: exilium 8 no        Sin prensentacion y 256 colores\n");
 printf("         exilium /?          Muestra esta ayuda\n");
 printf("         exilium setup       Carga el menu de configuracion\n\n");
 printf("Por defecto arranca en 16 bits de color y con presentacion\n");
}

/************************************************/
void GeneraClave(char *cadena){
//La clave es odioaloslamers y tengo que generarla para que no se vea en el
//binario
 cadena[0]='o';
 cadena[1]= (char)((int)(cadena[0])-11);
 cadena[2]= (char)((int)(cadena[1])+5);
 cadena[3]= cadena[0];
 cadena[4]= (char)((int)(cadena[1])-3);
 cadena[5]= (char)((int)(cadena[2])+3);
 cadena[6]= cadena[3];
 cadena[7]= (char)((int)(cadena[3])+4);
 cadena[8]= cadena[5];
 cadena[9]= cadena[4];
 cadena[10]=(char)((int)(cadena[8])+1);
 cadena[11]=(char)((int)(cadena[4])+4);
 cadena[12]=(char)((int)(cadena[6])+3);
 cadena[13]=(char)((int)(cadena[12])+1);
 cadena[14]='\0';
}

/************************************************/
/*void LeerArticuloDisco(char *nombre,TArticulo articulo){
 FILE *fichero;
 char *buffer;
 int i,tamanio, leidos;
 fichero = fopen(nombre,"rb");
 if (fichero!=NULL){
  fseek(fichero,0L,SEEK_END);
  tamanio = ftell(fichero);
  fseek(fichero,0L,SEEK_SET);
  buffer = (char *)malloc(tamanio);
  leidos = fread(buffer,sizeof(char),tamanio,fichero);
 }
 if (articulo!=NULL) free(articulo);
 articulo = buffer;
 fclose(fichero);
 printf("%d bytes\n",tamanio);
 for (i=0;i<tamanio;i++)
  printf("%c",articulo[i]);
}  */

/************************************************/
void AccionMenuDesplegable(){
//Muestra de forma visual el menu de los articulos
 if (menuArticulosDesplegado==FALSE){
  if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
  blit(configuracion[FOTO_BARRA].dat,videoTemporal,0,0,0,400,640,80);
  if (raton) show_mouse(NULL);
  for (i=0;i<42;i++){
   //blit(configuracion[FOTO_BARRA].dat,videoTemporal,168,16,168,416+i,292,35);
   //rect(screen,168,416,168+292,416+22,255);
   //rect(screen,260,400+39,370,400+39+11,255);
   blit(configuracion[FOTO_BARRA].dat,videoTemporal,168,16,168,416+i,292,23);
   blit(configuracion[FOTO_BARRA].dat,videoTemporal,260,39,260,439+i,110,12);
   vsync();
   blit(videoTemporal,screen,168,416,168,416,292,74);
   menuArticulosDesplegado = TRUE;
  }
  //Muestro los articulos a elegir
  MostrarArticulosPantalla(&losArticulos,letras);
  masked_blit(letras,videoTemporal,0,0,0,0,anchoX,altoY);
  blit(videoTemporal,screen,168,416,168,416,292,74);
  if (raton) show_mouse(screen);
 }
 else{
  //Recojo el menu desplegable de articulos
  if (raton) show_mouse(NULL);
  for (i=42;i>=0;i--){
   blit(configuracion[FOTO_BARRA].dat,videoTemporal,168,16+i,168,416+i,292,74-i);
   blit(configuracion[FOTO_BARRA].dat,videoTemporal,168,16,168,416+i,292,24);
   blit(configuracion[FOTO_BARRA].dat,videoTemporal,260,39,260,439+i,110,12);
   vsync();
   blit(videoTemporal,screen,168,416,168,416,292,74);
   menuArticulosDesplegado = FALSE;
  }
  if (raton) show_mouse(screen);
 }
}

/*********************************************************/
void AccionBarraDesplegable(){
 BITMAP *videoTemporal2 = NULL;
 BITMAP *videoTemporal3 = NULL;
 int i;
 
  videoTemporal2 = create_bitmap(640,80);
  videoTemporal3 = create_bitmap(640,80);

  if (raton) show_mouse(NULL);

  blit(screen,videoTemporal2,0,0,0,0,640,80);
  blit(screen,videoTemporal3,0,400,0,0,640,80);
  
  for (i=0;i<80;i++){
   vsync();
   //blit(videoTemporal2,screen,0,0,0,i,640,79-i);
   //line(screen,0,i,639,i,makecol(0,0,0));
   blit(videoTemporal2,screen,0,i,0,0,640,79-i);
   line(screen,0,79-i,640,79-i,color_negro);
   //blit(videoTemporal3,screen,0,i,0,400,640,79-i);
   //line(screen,0,479-i,639,479-i,makecol(0,0,0));
   blit(videoTemporal3,screen,0,0,0,400+i,640,80-i);
   line(screen,0,400+i,640,400+i,color_negro);
  }

  if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
    blit(configuracion[FOTO_BARRA].dat,screen,519,5,519,405,24,25);

  if (raton) show_mouse(screen);
  while (!((mouse_y>=408)&&(mouse_y<=429)&&(mouse_x>=521)&&(mouse_x<=542)&&(mouse_b & 1)));
  while (mouse_b != 0);

  if (raton) show_mouse(NULL);

  rectfill(screen,519,405,553,430,color_negro);
  
  clear_keybuf();
   
  for (i=0;i<80;i++){
   vsync();
   //blit (videoTemporal2,screen,0,0,0,79-i,640,i+1);
   //blit (videoTemporal3,screen,0,79-i,0,400,640,i+1);
   blit(videoTemporal2,screen,0,79-i,0,0,640,i+1);
   blit(videoTemporal3,screen,0,0,0,479-i,640,i+1);
  }
  if (raton) show_mouse(screen);
  destroy_bitmap(videoTemporal2);
  destroy_bitmap(videoTemporal3);
}

/************************************************************/
void ControlaEventoRaton(int *ratonX, int *ratonY, int *ratonZantes, boolean *salir){
int ratonZahora;
if (raton){
   get_mouse_mickeys(ratonX,ratonY);
   if ((mouse_b & 2)&&(*ratonY!=0)){
     if (menuArticulosDesplegado==FALSE){
       if (menuBuscar==FALSE){
        if (*ratonY>0) ScrollArriba();
        if (*ratonY<0) ScrollAbajo();
       }
     }
     else{
      if (menuBuscar==FALSE){
        if (*ratonY>0) ScrollAbajoArticulos(&losArticulos,letras);
        if (*ratonY<0) ScrollArribaArticulos(&losArticulos,letras);
      }
     }
   }

   //Para BeOS, WINDOWS y LINUX
   ratonZahora = mouse_z;
   if (ratonZahora != *ratonZantes){
     if (menuArticulosDesplegado==FALSE){
      if (menuBuscar==FALSE){
       if (ratonZahora > *ratonZantes) ScrollAbajo();
       if (ratonZahora < *ratonZantes) ScrollArriba();
      }
     }
     else{
      if (menuBuscar==FALSE){
       if (ratonZahora > *ratonZantes) ScrollArribaArticulos(&losArticulos,letras);
       if (ratonZahora < *ratonZantes) ScrollAbajoArticulos(&losArticulos,letras);
      }
     }
     *ratonZantes = ratonZahora;
   }
   
   if ((mouse_b & 4)&&(*ratonY!=0)&&(menuArticulosDesplegado!=FALSE)&&(menuBuscar==FALSE)){
     if (*ratonY>0) ScrollAbajoArticulos(&losArticulos,letras);
     if (*ratonY<0) ScrollArribaArticulos(&losArticulos,letras);
   }
   
   
   if (mouse_b & 1) {
     if ((mouse_y>=447)&&(mouse_y<=468)&&(mouse_x>=120)&&(mouse_x<=142)&&(driverSonido!=FALSE)){
      //Boton de sonido
      if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
      show_mouse(NULL); //Desplazamos boton visual de sonido
       blit(configuracion[FOTO_BARRA].dat,screen,115,39,116,440,35,35);
      show_mouse(screen);
      
      while (mouse_b!=0); //Se ha liberado el boton y restauramos
      show_mouse(NULL);
       blit(configuracion[FOTO_BARRA].dat,screen,115,39,115,439,36,36);
      show_mouse(screen);
     }
     else{
      if ((mouse_y>=410)&&(mouse_y<=436)&&(mouse_x>=96)&&(mouse_x<=124)){
       //Boton de busqueda
       if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
       show_mouse(NULL); //Desplazamos boton visual de busqueda
        blit(configuracion[FOTO_BARRA].dat,screen,94,5,95,406,35,35);
       show_mouse(screen);
      
       while (mouse_b!=0); //Se ha liberado el boton y restauramos
       show_mouse(NULL);
        blit(configuracion[FOTO_BARRA].dat,screen,94,5,94,405,36,36);
       show_mouse(screen);
      }
      else{
       if ((mouse_y>=415)&&(mouse_y<=470)&&(mouse_x>=18)&&(mouse_x<=70)){
        //Boton de cargar tema
        if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
        show_mouse(NULL); //Desplazamos boton visual de cargar tema
         blit(configuracion[FOTO_BARRA].dat,screen,25,22,26,423,39,33);
        show_mouse(screen);
      
        while (mouse_b!=0); //Se ha liberado el boton y restauramos
        show_mouse(NULL);
         blit(configuracion[FOTO_BARRA].dat,screen,25,22,25,422,40,34);
        show_mouse(screen);
       }
       else{
        if ((mouse_y>=420)&&(mouse_y<=464)&&(mouse_x>=570)&&(mouse_x<=620)){
         //Boton de salir de la diskmag
         if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
         show_mouse(NULL); //Desplazamos boton visual de salir
          blit(configuracion[FOTO_BARRA].dat,screen,583,17,584,418,36,42);
         show_mouse(screen);

         while (mouse_b!=0); //Se ha liberado el boton de salir
         show_mouse(NULL);
          blit(configuracion[FOTO_BARRA].dat,screen,583,17,583,417,37,43);
         show_mouse(screen);         
        }
        else{
         if ((mouse_y>=454)&&(mouse_y<=469)&&(mouse_x>=495)&&(mouse_x<=511)){
          //BOTON de MUSICA izquierda
          if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
          show_mouse(NULL); //Desplazamos boton visual de izquierda musica
           blit(configuracion[FOTO_BARRA].dat,screen,494,54,493,455,20,16);
          show_mouse(screen);

          while (mouse_b!=0); //Se ha liberado el boton de izquierda musica
          show_mouse(NULL);
           blit(configuracion[FOTO_BARRA].dat,screen,493,54,493,454,21,17);
          show_mouse(screen);          
         }
         else{
          if ((mouse_y>=454)&&(mouse_y<=469)&&(mouse_x>=524)&&(mouse_x<=540)){
           //BOTON de MUSICA derecho
           if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
           show_mouse(NULL); //Desplazamos boton visual de derecha musica
            blit(configuracion[FOTO_BARRA].dat,screen,524,53,525,454,16,16);
           show_mouse(screen);

           while (mouse_b!=0); //Se ha liberado el boton de derecha musica
           show_mouse(NULL);
            blit(configuracion[FOTO_BARRA].dat,screen,524,53,524,453,17,17);
           show_mouse(screen);
          }
          else{          
           if ((mouse_y>=434)&&(mouse_y<=445)&&(mouse_x>=295)&&(mouse_x<=335)&&(menuArticulosDesplegado==FALSE)){
            //BOTON de sacar menu desplegable
            if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
            show_mouse(NULL); //Desplazamos boton visual de menu
             blit(configuracion[FOTO_BARRA].dat,screen,295,33,296,434,40,15);
            show_mouse(screen);

            while (mouse_b!=0); //Se ha liberado el boton menu
            show_mouse(NULL);
             blit(configuracion[FOTO_BARRA].dat,screen,295,33,295,433,41,16);
            show_mouse(screen);
           }
           else{
            if ((mouse_y>=474)&&(mouse_y<=479)&&(mouse_x>=295)&&(mouse_x<=335)&&(menuArticulosDesplegado!=FALSE)){
             //BOTON de sacar menu desplegable para quitarlo
             if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
             show_mouse(NULL); //Desplazamos boton visual de menu quitar
              blit(configuracion[FOTO_BARRA].dat,screen,295,33,296,473,40,15);
             show_mouse(screen);

             while (mouse_b!=0); //Se ha liberado el boton menu quitar
             show_mouse(NULL);
              blit(configuracion[FOTO_BARRA].dat,screen,295,33,295,472,41,16);
             show_mouse(screen);
            }
            else{
             if ((mouse_y>=460)&&(mouse_y<=467)&&(mouse_x>=447)&&(mouse_x<=454)&&(menuArticulosDesplegado!=FALSE)){
              //BOTON de scroll articulos arriba
              if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
              show_mouse(NULL); //Desplazamos boton visual de scroll arriba
               blit(configuracion[FOTO_BARRA].dat,screen,447,18,448,458,6,8);
              show_mouse(screen);

              while (mouse_b!=0); //Se ha liberado el boton de scroll
              show_mouse(NULL);
               blit(configuracion[FOTO_BARRA].dat,screen,447,18,447,459,7,9);
              show_mouse(screen);
             }
             else{
              if ((mouse_y>=467)&&(mouse_y<=477)&&(mouse_x>=447)&&(mouse_x<=454)&&(menuArticulosDesplegado!=FALSE)){
               //BOTON de scroll articulos abajo
               if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
               show_mouse(NULL); //Desplazamos boton visual de scroll abajo
                blit(configuracion[FOTO_BARRA].dat,screen,447,26,448,467,6,8);
               show_mouse(screen);

               while (mouse_b!=0); //Se ha liberado el boton de scroll
               show_mouse(NULL);
                blit(configuracion[FOTO_BARRA].dat,screen,447,26,447,467,7,9);
               show_mouse(screen);
              }
              else{
               if ((mouse_y>=465)&&(mouse_y<=477)&&(mouse_x>=175)&&(mouse_x<=182)&&(menuArticulosDesplegado!=FALSE)){
                //BOTON de cargar articulo en menu
                if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
                show_mouse(NULL);
                 blit(configuracion[FOTO_BARRA].dat,screen,174,23,175,465,9,8);
                show_mouse(screen);

                while (mouse_b!=0);
                show_mouse(NULL);
                 blit(configuracion[FOTO_BARRA].dat,screen,174,23,174,464,10,9);
                show_mouse(screen);                
               }
               else{
               if ((mouse_y>=408)&&(mouse_y<=429)&&(mouse_x>=521)&&(mouse_x<=542)){
                 //BOTON de minimizar
                 if (!grises) select_palette(configuracion[PALETA_BARRA].dat);
                 show_mouse(NULL);
                  blit(configuracion[FOTO_BARRA].dat,screen,520,6,521,407,22,23);
                 show_mouse(screen);

                 while (mouse_b!=0);
                 show_mouse(NULL);
                  blit(configuracion[FOTO_BARRA].dat,screen,520,6,520,406,23,24);
                 show_mouse(screen);
                }
               }
              }
             }
            }
           }
          }
         }
        }
       }
      }
     }
     
     if ((mouse_y>=447)&&(mouse_y<=468)&&(mouse_x>=120)&&(mouse_x<=142)&&(driverSonido!=FALSE)) AccionSonido();
     else{
      if ((mouse_y>=434)&&(mouse_y<=445)&&(mouse_x>=295)&&(mouse_x<=335)&&(menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE)) AccionMenuDesplegable();
      else{
       if ((mouse_y>=474)&&(mouse_y<=479)&&(mouse_x>=295)&&(mouse_x<=335)&&(menuArticulosDesplegado!=FALSE)&&(menuBuscar==FALSE)) AccionMenuDesplegable();
       else{
        if ((mouse_y>=460)&&(mouse_y<=467)&&(mouse_x>=447)&&(mouse_x<=454)&&(menuArticulosDesplegado!=FALSE)) ScrollArribaArticulos(&losArticulos,letras);
        else{
         if ((mouse_y>=467)&&(mouse_y<=477)&&(mouse_x>=447)&&(mouse_x<=454)&&(menuArticulosDesplegado!=FALSE)) ScrollAbajoArticulos(&losArticulos,letras);
         else{
          if ((mouse_y>=465)&&(mouse_y<=477)&&(mouse_x>=175)&&(mouse_x<=182)&&(menuArticulosDesplegado!=FALSE)&&(menuBuscar==FALSE)){
            AccionMenuDesplegable();
            EliminarArticulo();
            InicializarArticulo();
            MostrarArticulo(losArticulos.articulo[(losArticulos.articuloActual)]);          
          }
          else{
           if ((mouse_y>=420)&&(mouse_y<=430)&&(mouse_x>=192)&&(mouse_x<=436)&&(menuArticulosDesplegado!=FALSE)&&(menuBuscar==FALSE)){
             AccionMenuDesplegable();
             EliminarArticulo();
             InicializarArticulo();
             MostrarArticulo(losArticulos.articulo[(losArticulos.articuloActual)]);
           }
           else{
            if ((mouse_y>=408)&&(mouse_y<=429)&&(mouse_x>=521)&&(mouse_x<=542)) AccionBarraDesplegable();
            else{
             if ((mouse_y>=415)&&(mouse_y<=470)&&(mouse_x>=18)&&(mouse_x<=70)&&(menuArticulosDesplegado!=FALSE)&&(menuBuscar==FALSE)){
              AccionMenuDesplegable();
              EliminarArticulo();
              InicializarArticulo();
              MostrarArticulo(losArticulos.articulo[(losArticulos.articuloActual)]);
             }
             else{
              if ((mouse_y>=410)&&(mouse_y<=436)&&(mouse_x>=96)&&(mouse_x<=124)) AccionBuscar(salir);
              else{
               if ((mouse_y>=420)&&(mouse_y<=464)&&(mouse_x>=570)&&(mouse_x<=620)){
                while (mouse_b!=0); //Se ha liberado el boton
                *salir = TRUE;
               }
               else{
                if ((mouse_y>=454)&&(mouse_y<=469)&&(mouse_x>=495)&&(mouse_x<=511)){
                 numMidiActual--;
                 SiguienteCancion();
                }
                else{
                 if ((mouse_y>=454)&&(mouse_y<=469)&&(mouse_x>=524)&&(mouse_x<=540)){
                  numMidiActual++;
                  SiguienteCancion();
                 }
                }
               }
              }
             }
            }            
           }
          }
         }
        }
       }
      }
  /*    else{
       EliminarArticulo();
       InicializarArticulo();
       MostrarArticulo(losArticulos.articulo[(losArticulos.articuloActual)]);
      } */
     }
   }
  }
}

/**************************************************************/
void ControlaEventoTeclado(boolean * salir){
 int scancode; 
 if (keypressed()){
  scancode = readkey()>>8;
  switch(scancode){
   case KEY_ENTER: if (menuArticulosDesplegado!=FALSE){
                    AccionMenuDesplegable();
                    EliminarArticulo();
                    InicializarArticulo();
                    MostrarArticulo(losArticulos.articulo[(losArticulos.articuloActual)]);
                   }
                   break;
   case KEY_ESC: *salir = TRUE; break;
   case KEY_UP: if ((menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE))
                  ScrollAbajo();
                else
                  ScrollArribaArticulos(&losArticulos,letras);
                break;
   case KEY_DOWN: if ((menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE))
                    ScrollArriba();
                  else
                    ScrollAbajoArticulos(&losArticulos,letras);
                  break;
   case KEY_PGUP: if ((menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE)) ScrollAbajo30Lineas(); break;
   case KEY_PGDN: if ((menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE)) ScrollArriba30Lineas(); break;
   case KEY_LEFT: if (menuArticulosDesplegado !=FALSE) ScrollArribaArticulos(&losArticulos,letras); break;
   case KEY_RIGHT: if (menuArticulosDesplegado !=FALSE) ScrollAbajoArticulos(&losArticulos,letras); break;
   case KEY_F12: save_pcx("screen.pcx",videoTemporal,NULL); break;
   case KEY_F10: save_pcx("screen.pcx",videoTemporal,NULL); break;
   case KEY_PLUS_PAD: ReguladorVolumen(mas); break;
   case KEY_MINUS_PAD: ReguladorVolumen(menos); break;
   case KEY_S: AccionSonido(); break;
   case KEY_M: if (menuBuscar==FALSE) AccionMenuDesplegable(); break;
   case KEY_F3: AccionBuscar(salir); break;   
   case KEY_END: if ((menuBuscar==FALSE)&&(menuArticulosDesplegado==FALSE)){
                  elArticulo.posFila = (elArticulo.numFilas - 1 - (elArticulo.numFilas%30))-5;
                  ScrollArriba30Lineas();
                 }
                 break;
   case KEY_HOME: if ((menuBuscar==FALSE)&&(menuArticulosDesplegado==FALSE)){
                   elArticulo.posFila = -1;
                   ScrollArriba30Lineas();
                  }
                  break;
   case KEY_A: numMidiActual--; SiguienteCancion(); break;
   case KEY_D: numMidiActual++; SiguienteCancion(); break;
  }
  clear_keybuf();
 }
}

/************************************************************/
void ControlaEventoJoystick(){
 int numeroBotones;
 if (palancaJuegos!=FALSE){
  numeroBotones = joy[0].num_buttons;
  poll_joystick();

   //Es joystick digital
    //Se ha pulsado la izquierda
   if (joy[0].stick[0].axis[0].d1){
//    if ((menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE))
     ScrollAbajo30Lineas();
   }

/*    crono = 0;
    LOCK_VARIABLE(crono);
    LOCK_FUNCTION(inc_crono);
    install_int(inc_crono,500); //Arranco el temporizador de segundos 
    while ((joy[0].stick[0].axis[0].d1)&&(crono<1))
     poll_joystick(); //Espera a que se libere

    while (joy[0].stick[0].axis[0].d1){
     poll_joystick();
     if ((menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE))
      ScrollAbajo30Lineas();     
    }
    remove_int(inc_crono);
   }   */

/*   if (joy[0].stick[0].axis[0].d2){
    //Derecha
    if ((menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE))
     ScrollArriba30Lineas();     

    crono = 0;
    LOCK_VARIABLE(crono);
    LOCK_FUNCTION(inc_crono);
    install_int(inc_crono,500); //Arranco el temporizador de segundos
    while ((joy[0].stick[0].axis[0].d2)&&(crono<1))
     poll_joystick(); //Espera a que se libere
     while (joy[0].stick[0].axis[0].d2){
     poll_joystick();
     if ((menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE))
      ScrollArriba30Lineas();
    }
    remove_int(inc_crono);
   }*/

/*   if (joy[0].stick[0].axis[1].d1){
    //Boton Arriba
    if ((menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE))
     ScrollAbajo();
    else
     ScrollArribaArticulos(&losArticulos,letras);

    crono = 0;
    LOCK_VARIABLE(crono);
    LOCK_FUNCTION(inc_crono);
    install_int(inc_crono,500); //Arranco el temporizador de segundos
    while ((joy[0].stick[0].axis[1].d1)&&(crono<1)) poll_joystick(); //Espera a que se libere

    while (joy[0].stick[0].axis[1].d1){
     poll_joystick();
     if ((menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE))
      ScrollAbajo();
     else
      ScrollArribaArticulos(&losArticulos,letras);
    }
    remove_int(inc_crono);
   }

   if (joy[0].stick[0].axis[1].d2){
    //Boton Abajo
    if ((menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE))
     ScrollArriba();
    else
     ScrollAbajoArticulos(&losArticulos,letras);

    crono = 0;
    LOCK_VARIABLE(crono);
    LOCK_FUNCTION(inc_crono);
    install_int(inc_crono,500); //Arranco el temporizador de segundos
    while ((joy[0].stick[0].axis[1].d2)&&(crono<1))
     poll_joystick(); //Espera a que se libere

    while (joy[0].stick[0].axis[1].d2){
     poll_joystick();
     if ((menuArticulosDesplegado==FALSE)&&(menuBuscar==FALSE))
      ScrollArriba();
     else
      ScrollAbajoArticulos(&losArticulos,letras);
    }
    remove_int(inc_crono);
   }*/

   //Miramos los botones del joystick

/*   if (numeroBotones>=0){
    if ((joy[0].button[0].b)&&(menuBuscar==FALSE))
     AccionMenuDesplegable(); //Accion Menu
   }

   if ((numeroBotones>=1)){
    if ((joy[0].button[1].b)&&(menuArticulosDesplegado!=FALSE)){
     AccionMenuDesplegable();
     EliminarArticulo();
     InicializarArticulo();
     MostrarArticulo(losArticulos.articulo[(losArticulos.articuloActual)]);
    }
    while (joy[0].button[1].b) poll_joystick();  //Accion cargar tema    
   }

   if ((numeroBotones>=2)){
    if (joy[0].button[2].b){
      numMidiActual--;
      SiguienteCancion();
    }
   }

   if ((numeroBotones>=3)){
    if (joy[0].button[3].b){
      numMidiActual++;
      SiguienteCancion();
    }
   }*/
 }
}

/************************************************************/
boolean ComparaCadena(char *cad1, char *cad2){
 //Compara las cadenas diciendo:
 //  TRUE  si la cad2 esta contenida en cad1
 //  FALSE en caso contrario
 short int i,j, longitudCad1, longitudCad2;
 char cadAux1[100];
 char cadAux2[100];
 boolean encontrada = FALSE;
 if ((cad1!=NULL) && (cad2!=NULL)){
  strcpy(cadAux1,cad1);
  strcpy(cadAux2,cad2);  
  longitudCad1 = strlen(cadAux1);
  longitudCad2 = strlen(cadAux2);
  for (i=0;i<longitudCad1;i++)
   switch (cadAux1[i]){
    case ' ': cadAux1[i] = 'A'; break;
    case '‚': cadAux1[i] = 'E'; break;
    case '¡': cadAux1[i] = 'I'; break;
    case '¢': cadAux1[i] = 'O'; break;
    case '£': cadAux1[i] = 'U'; break;
    case '': cadAux1[i] = 'U'; break;
    case 'µ': cadAux1[i] = 'A'; break;
    case '': cadAux1[i] = 'E'; break;
    case 'Ö': cadAux1[i] = 'I'; break;
    case 'à': cadAux1[i] = 'O'; break;
    case 'é': cadAux1[i] = 'U'; break;
    case 'š': cadAux1[i] = 'U'; break;
   }
  strupr(cadAux1);
  strupr(cadAux2);
  
  i = 0;
  while ((i<longitudCad1)&&(encontrada==FALSE)){
   if (cadAux1[i]==cadAux2[0]){
    j=0;
    while ((cadAux1[i]==cadAux2[j])&&(i<longitudCad1)&&(j<longitudCad2)){
     i++;
     j++;
    }
    if (longitudCad2==j) encontrada = TRUE;
   }
   else i++;
  }
 return (encontrada);
 }
 else return (FALSE);
}
/************************************************************/
void AccionBuscar(boolean *salir){
//Busca una palabra en el documento
 int ratonX = 0;
 int ratonY = 0;
 int ratonZantes = 0;
 int scancode,i,anterior;
 int valorAscii,aux;
 Ttexto * pAux = NULL;
 boolean salir2 = FALSE;
 BITMAP *videoTemporal2 = NULL;
 BITMAP *videoTemporal3 = NULL;
 char cadena[longitud_cadena];

 if (menuBuscar==FALSE){
  menuBuscar = TRUE;
  if (menuArticulosDesplegado!=FALSE) AccionMenuDesplegable();
   videoTemporal2 = create_bitmap(155,20);
   videoTemporal3 = create_bitmap(162,17);
   anterior = text_mode(color_blanco); //Pongo a blanco el fondo de las letras
   clear_to_color(videoTemporal2,color_blanco);
   if (raton) show_mouse(NULL);
   blit(screen,videoTemporal3,240,460,0,0,161,16);
   rectfill(screen,240,460,400,475,color_blanco);
   rect(screen,241,461,399,474,color_negro);
   textout(videoTemporal2, datafile[FUENTE_CHAR14B].dat,"Palabra:",0,0,color_negro);
   blit(videoTemporal2,screen,1,1,246,462,100,11);
   if (raton) show_mouse(screen);

   i = 0;
   cadena[0]='\0';
   while ((salir2==FALSE)&&(*salir==FALSE)){
    text_mode(anterior);
    ControlaEventoRaton(&ratonX, &ratonY, &ratonZantes, salir);
    text_mode(color_blanco);
    if (keypressed()){
     scancode = readkey();
     if ((scancode>>8) != KEY_ENTER){
      if (((scancode>>8) == KEY_LEFT || (scancode>>8) == 0x3F)&&(i>=1)) i-=2;
      else{
       if (i>11) i=11;       
       valorAscii = scancode & 0xFF;
       if (valorAscii==59) cadena[i] = (char)'¤';
       else{
        if ((valorAscii >=  'a') && (valorAscii<='z'))
           cadena[i] = (char)(scancode & 0xFF);
        else{
         if ((valorAscii >=  '0') && (valorAscii<='9'))
           cadena[i] = (char)(scancode & 0xFF);
         else
           i--;
        }
       }
      }
      i++;
      cadena[i] = '\0';
      rectfill(videoTemporal2,55,1,199,12,color_blanco);
      textout(videoTemporal2, datafile[FUENTE_PCT].dat,cadena,55,4,color_negro);
      if (raton) show_mouse(NULL);
       blit(videoTemporal2,screen,1,1,246,462,150,11);
      if (raton) show_mouse(screen);
     }
     else salir2 = TRUE;
    }
   }

  if (salir2!=FALSE){
   //Se ha puesto la palabra y hay que buscarla
   i = elArticulo.posFila;
   if (i==29) i=0;
   if (i>29) i=i-30;
   salir2 = FALSE;
   while ((i<elArticulo.numFilas)&&(salir2==FALSE)){
    //Busco a partir de la posicion actual
    if (ComparaCadena(elArticulo.info[i].cadena,cadena)!=FALSE) salir2 = TRUE;    
    else{
     if (elArticulo.info[i].sig!=NULL){
      pAux = elArticulo.info[i].sig;
      while ((pAux!=NULL)&&(salir2==FALSE)){
       if (ComparaCadena(pAux->cadena,cadena)!=FALSE) salir2 = TRUE;
       else pAux = (Ttexto *)pAux->sig;
      }
     }
     if (salir2==FALSE) i++;
    }
   }
   if ((elArticulo.posFila>0)&&(salir2==FALSE)){
    //Recorro desde el principio
    i=0;
    while ((i<elArticulo.posFila)&&(salir2==FALSE)){
     if (ComparaCadena(elArticulo.info[i].cadena,cadena)!=FALSE) salir2 = TRUE;     
     else{
      if (elArticulo.info[i].sig!=NULL){
       pAux = elArticulo.info[i].sig;
       while ((pAux!=NULL)&&(salir2==FALSE)){
        if (ComparaCadena(pAux->cadena,cadena)!=FALSE) salir2 = TRUE;
        else pAux = (Ttexto *)pAux->sig;
       }
      }
      if (salir2==FALSE) i++;
     }
    }
   }
   if (salir2!=FALSE){
    //He encontrado la palabra
    aux = i;
    if (aux<0) aux = 0;
    i = i-1-(i%5);
    if (i<=0) i=-1;
    elArticulo.posFila = i;
    text_mode(anterior);    
    ScrollArriba30Lineas();
    //drawing_mode(DRAW_MODE_TRANS,NULL,0,0);
    xor_mode(TRUE);
     aux = 89+ ((aux%5)*10);
     if (raton) show_mouse(NULL);
      rectfill(screen,3,aux,636,140,color_blanco);
     if (raton) show_mouse(screen);
    xor_mode(FALSE);
   }   
  }

  text_mode(anterior);
  if (raton) show_mouse(NULL);
   blit(videoTemporal3,screen,0,0,240,460,161,16);
  if (raton) show_mouse(screen);
  destroy_bitmap(videoTemporal2);
  destroy_bitmap(videoTemporal3);
  clear_keybuf();
  menuBuscar = FALSE;
 }
}


