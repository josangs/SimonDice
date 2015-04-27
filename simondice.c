/* ************************************************************************** */
/* PROGRAMA    : Simondice.c (v1.0)                                           */
/* TARGET      : Donde se va a ejecutar el código(ej. MSP430F169 en simulador)*/
/* DESCRIPCION : Implementación Hardware del Juego SimonDice                  */
/* AUTOR       : José Antonio Garrido Siles                                   */
/* FECHA       : 15-01-2014                                                   */
/* ESQUEMA     : ACLK= 0 Hz MCLK= SMCLK= default DCO ~ 1MHz


                MSP430G2553
             -----------------                              
            |                 |             
         +--|XIN              |             
   XTAL [ ] |                 |             
         +--|XOUT             |            
            |                 |
            |                 |
            |		      |150 Ohm		      
            |             P1.0|-/\/\/\--|>|-----------+ LED(BIT0)
            |                 |                  GND ---
            |                 |                       -
            |                 |             o Vcc
            |                 |             |
            |                 |            \
            |                 |             \  1 MOhm
            |             P1.3|-------------+-/\/\/\--+       PULSADOR 1 (S2)
            |                 |                       |      
            |                 |                       |  
	    |		      |		         GND ---
            |                 |                       -
            |                 |
            |                 | 150 Ohm               
            |             P1.5|-/\/\/\--|>|-----------+ LED(BIT5)
            |                 | 150 Ohm               |
            |             P1.6|-/\/\/\--|>|-----------+ LED(BIT6)
            |                 |  		GND  ---
            |                 |                       - 
            |                 |  		      
            |                 |             o Vcc
            |                 |             |
            |                 |             |
  	    |                 |            \
            |                 |             \  1 MOhm
            |             P2.0|-------------+-/\/\/\--+      PULSADOR 2 (S3)
            |                 |                       |     
            |                 |                       |    
	    |		      |		         GND ---
            |                 |  		      -
            |                 |              o Vcc
            |                 |              |
            |                 |              |
            |                 |              |      
            |             P2.1|--------------+-/\/\/\--+    PULSADOR 3 (S4)
            |                 |                        |
            |                 |                        |
            |                 |                    GND---                                 
            |                 |                        -
            |                 |              
            |                 |             
            |                 |                                               */
/* ************************************************************************** */

#include "io430g2553.h"
#include "intrinsics.h"
#include "stdlib.h"
#include "iso646.h"

/* ====== CONSTANTES ======================================================== */

#define ON 1                         //Activo
#define OFF 0                        //Desactivo
#define PULS_ON 0                    //Pulsador Pulsado
#define PULS_OFF 1                   //Pulsador Soltado
#define LED_ON  1                    //Led encendido
#define LED_OFF 0                    //Led apagado
#define tam 10                       //Tamaño de la cadena

/* ====== TIPOS DATOS ======================================================= */

typedef union{
  unsigned volatile char total;   //Todos los flags
  struct{
    unsigned char f0 :1;    //Flag 0
    unsigned char f1 :1;    //Flag 1
    unsigned char f2 :1;    //Flag 2
    unsigned char f3 :1;    //Flag 3
    unsigned char f4 :1;    //Flag 4
    unsigned char f5 :1;    //Flag 5
  };
} flags;

/* ====== VARIABLES GLOBALES ================================================ */

flags tareas;         //Flags activacion tareas:
                     //BIT0: Tarea_0 -> Pulsador S2 presionado
                     //BIT1: Tarea_1 -> Pulsador S2 soltado
                    //BIT2: Tarea_2 -> Pulsador S3 presionado
                    //BIT3: Tarea_3 -> Pulsador S3 soltado
unsigned int has_perdido;    // False = 1 --> La secuencia es buena.
                           // True = 0 --> La secuencia se ha fallado.
unsigned int sec[tam];   //array que guarda teclas que se han pulsado.
//unsigned int sal;
unsigned int cont_inicio,cont_medio,cont_final;
unsigned int inicio,tiempo;
unsigned int aleatorio, dir_lectura, dir_escritura;
unsigned int leido, color_leido, color_pulsado, aciertos;



/* ====== PROTOTIPOS ======================================================== */

void Config_uC(void);
void Config_Puertos(void);
void Config_Timers(void);

//Subfunciones
void genera_aleatorio(void);
void guarda_color(void);
void muestra_secuencia(void);
void pulsa_secuencia(void);

/* ====== PROGRAMA PRINCIPAL ================================================ */

int main(void){
  Config_uC();
  Config_Puertos();
  Config_Timers();
  tareas.total= 0;
  has_perdido = 0;
  aleatorio = 0;
  aciertos = 0;
  dir_lectura= 0;
  dir_escritura = 0;
  color_leido = 0;
  color_pulsado = 0;
  inicio = 0; tiempo = 0;
  
  __enable_interrupt();   //Habilita interrupciones
  
  for(;;){
    
    __low_power_mode_0();    //Bajo consumo (LPM0)
    while(tareas.total != OFF){
      //Parte Principal.
      if(inicio==1){
        if(cont_inicio == 6){
          P1OUT_bit.P0 = LED_ON;
        }// if (cont == 1)
        else if(cont_inicio == 12){
          P1OUT_bit.P5 = LED_ON;
          P1OUT_bit.P0 = LED_OFF;
        }// else if (cont_inicio == 2)
        else if(cont_inicio == 18){
          P1OUT_bit.P6 = LED_ON;
          P1OUT_bit.P5 = LED_OFF;
        }// else if (cont_inicio == 3)
        //Espera un 1s para mostrar la secuencia.
        else if(cont_inicio == 36){
          P1OUT_bit.P6 = LED_OFF;
            inicio = 0;
            TA0CCTL0_bit.CCIE = OFF;
            cont_inicio = 0;
            has_perdido = 0;
        }// else if(cont_inicio == 36)
      }// if(inicio == 1)
      
      if(inicio == 0){
        tareas.f0 = OFF; tareas.f2 = OFF; tareas.f4 = OFF;
        has_perdido = 0;
        //Activo Timer.
        TA0CTL_bit.TACLR = ON;
        TA0CCTL0_bit.CCIE = ON;
        //Activo Timer.
        cont_final = 0;
        while(cont_final<=18){
          //Sistema espera un segundo.
        }//while(cont_final<=18)
        
        while(has_perdido == 0){
          genera_aleatorio();
          guarda_color();
          muestra_secuencia();
          pulsa_secuencia();
          if(has_perdido == 0){
            cont_final = 0;
            aciertos++;
            P1OUT_bit.P0 = P1OUT_bit.P5 = P1OUT_bit.P6 = OFF;
            while(cont_final<=18){
              //Sistema espera un segundo.
            }//while(cont_final<=6)
          }//if(has_perdido == 0)
        }// while(has_perdido == 0)
        cont_final = 0;
        P1OUT &= ~(BIT0 + BIT5 + BIT6);
        while(has_perdido == 1){   //CORREGUIR ESTA RUTINA
            //Los led parpadean durante 5s, pero no me lo hace.
            if(cont_final==0){
              P1OUT_bit.P0 = LED_ON;
              P1OUT_bit.P5 = LED_ON;
              P1OUT_bit.P6 = LED_ON;
            }//if(cont_final == 0)
            else if(cont_final == 18){
              P1OUT_bit.P0 = LED_OFF;
              P1OUT_bit.P5 = LED_OFF;
              P1OUT_bit.P6 = LED_OFF;
            }//else if(cont_final == 18)
            else if(cont_final == 36){
              P1OUT_bit.P0 = LED_ON;
              P1OUT_bit.P5 = LED_ON;
              P1OUT_bit.P6 = LED_ON;
            }//else if(cont_final == 36)
            else if(cont_final == 54){
              P1OUT_bit.P0 = LED_OFF;
              P1OUT_bit.P5 = LED_OFF;
              P1OUT_bit.P6 = LED_OFF;
            }//else if(cont_final == 54)
            else if(cont_final == 72){
              P1OUT_bit.P0 = LED_ON;
              P1OUT_bit.P5 = LED_ON;
              P1OUT_bit.P6 = LED_ON;
            }//else if(cont_final == 72)
            else if(cont_final == 90){
              P1OUT_bit.P0 = LED_OFF;
              P1OUT_bit.P5 = LED_OFF;
              P1OUT_bit.P6 = LED_OFF;
              aciertos = 0;
              dir_escritura = 0;
              has_perdido = 0;
              TA0CCTL0_bit.CCIE = OFF;
            }//else if(cont_final == 90)
          }// while(has_perdido == 1)
      }//if(inicio == 0)
   }//while(tareas.total!=OFF)
  }// for(;;)
}// int main (void)
    
/* ====== FUNCIONES ========================================================= */

void Config_uC(void){
  
        WDTCTL = WDTPW + WDTHOLD;   //Detiene watchdog
        DCOCTL = 0;                 //Frecuencia DCO (1MHz)
        BCSCTL1 = CALBC1_1MHZ;
        DCOCTL = CALDCO_1MHZ;

} // void Config_uC(void)

void Config_Puertos(void) {
  
// ----------------[Pulsador S2]
    
        P1DIR_bit.P3 = OFF;                //P1.3 --> Entrada (Pulsador S2 placa)
        P1REN_bit.P3 = ON;                 //P1.3 --> Resistencia activa
        P1OUT_bit.P3 = ON;                //P1.3 --> Resistencia pullup
        P1IES_bit.P3 = ON;                //P1.3 --> Flanco de bajada.
        P1IFG_bit.P3 = OFF;               //Limpia flag P1.3
        P1IE_bit.P3 = ON;                 //Habilita interrupcion P1.3
        
// ----------------[Pulsador S3]
        
        P2DIR_bit.P0 = OFF;               //P2.0 --> Entrada (Pulsador S3 proto)
        P2REN_bit.P0 = ON;                //P2.0 --> Resistencia activa  
        P2OUT_bit.P0 = ON;                //P2.0 --> Resistencia pullup
        P2IES_bit.P0 = ON;                //P2.0 --> Flanco de bajada
        P2IFG_bit.P0 = OFF;               //Limpia flag P2.0
        P2IE_bit.P0 = ON;                 //Habilita interrupcion P2.0
  
// ----------------[Pulsador S4]
        
        P2DIR_bit.P1 = OFF;               //P2.1 --> Entrada (Pulsador S4 proto)
        P2REN_bit.P1 = ON;                //P2.1 --> Resistencia activa
        P2OUT_bit.P1 = ON;                //P2.1 --> Resistencia pullup
        P2IES_bit.P1 = ON;                //P2.1 --> Flanco de bajada
        P2IFG_bit.P1 = OFF;               //Limpia flag P2.1
        P2IE_bit.P1 = ON;                 //Habilita interrupcion P2.1
        
// ---------------- [LEDs]
        
        P1DIR_bit.P0 = P1DIR_bit.P5 = P1DIR_bit.P6 = ON;   //P1.0, P1.5 y P1.6 --> Salida 
                                                           //Rojo, Amarillo y Verde.
        P1OUT_bit.P0 = LED_ON;              //P1.0 --> Encendido ROJO
        P1OUT_bit.P5 = LED_ON;              //P1.5 --> Encendido AMARILLO
        P1OUT_bit.P6 = LED_ON;              //P2.4 --> Encendido VERDE.
       
} // void Config_Puertos(void)

void Config_Timers(void) {
  
  TA0CTL = TASSEL_2 + MC_1 + TACLR ;   // Selecciono fuente 1MHz y modo Up to CCR0.
  TA0CCR0 = 0xFFFF;   //Contamos hasta 65ms para lo primera parte del programa.
  TA0CCTL0_bit.CCIFG=OFF; //Borra el flag de interrupcion.
} // void Config_Timers(void)

void genera_aleatorio(void){
  aleatorio = rand()%3; // El aletario es 0,1,2.
} // void genera_aleatorio()

void guarda_color(void){ //Guarda el color generado 
  sec[dir_escritura]=aleatorio;
  dir_escritura++;
} // void guarda_color()

void muestra_secuencia(void){   //Desde el primero hasta el último que se ha guardado 
  dir_lectura = 0;              //en memoria, los vamos mostrando.
  cont_medio = 0;
  while(dir_lectura < dir_escritura){
    color_leido = sec[dir_lectura];
    if(color_leido == 0){
      while(cont_medio<=36){
        P1OUT_bit.P5 = LED_OFF;
        P1OUT_bit.P6 = LED_OFF;
        P1OUT_bit.P0 = LED_ON;
        if(cont_medio == 36){
          P1OUT_bit.P0 = LED_OFF;
        }//if(cont_medio == 36)
      }//while(cont_medio<=36)
    }//if(color_leido == 0)
    else if(color_leido == 1){
      while(cont_medio<=36){
        P1OUT_bit.P0 = LED_OFF;
        P1OUT_bit.P6 = LED_OFF;
        P1OUT_bit.P5 = LED_ON;
        if(cont_medio == 36){
          P1OUT_bit.P5 = LED_OFF;
        }//if(cont_media == 36)
      }//while(cont_medio<=36)
    }//else if(color_leido == 1)
    else if(color_leido == 2){
      while(cont_medio<=36){
        P1OUT_bit.P6 = LED_ON;
        P1OUT_bit.P0 = LED_OFF;
        P1OUT_bit.P5 = LED_OFF;
        if(cont_medio == 36){
          P1OUT_bit.P6 = LED_OFF;
        }//if(cont_media == 33)
      }//while(cont_medio<=36)
    }//else if(color_leido == 2)
    dir_lectura++;
    cont_medio = 0;  //Inicializamos para que funcione bien la cuenta.
  }// while(dir_lectura < dir_escritura)
} // void muestra_colores()


void pulsa_secuencia(void){  //Recogemos las pulsaciones y se va compobando si son 
                        //correctas hasta que alguna no lo sea o hasta que haya-
  dir_lectura = 0;     //mos acertado todos los colores guardados hasta el 
  cont_medio = 0;      //medio.
  while((dir_lectura < dir_escritura) && (has_perdido==0)){                 
    P1OUT_bit.P0 = P1OUT_bit.P5 = P1OUT_bit.P6 = OFF;
    tiempo = 1;
  __low_power_mode_0();
    leido = sec[dir_lectura];
    if(leido != color_pulsado){
      has_perdido = 1;
    }//if (leido != color_pulsado)
    if(color_pulsado == 0 && tareas.f0 == ON){
      tareas.f0 = OFF;
      while(cont_medio<=8){
        P1OUT_bit.P5 = LED_OFF;
        P1OUT_bit.P6 = LED_OFF;
        P1OUT_bit.P0 = LED_ON;
        if(cont_medio == 8){
          P1OUT_bit.P0 = LED_OFF;
        }//if(cont_medio == 8)
      }//while(cont_medio<=8)
    }//if(color_pulsado == 0)
    else if(color_pulsado == 1 && tareas.f2 == ON){
      tareas.f2 = OFF;
      while(cont_medio<=8){
        P1OUT_bit.P0 = LED_OFF;
        P1OUT_bit.P6 = LED_OFF;
        P1OUT_bit.P5 = LED_ON;
        if(cont_medio == 8){
          P1OUT_bit.P5 = LED_OFF;
        }//if(cont_media == 8)
      }//while(cont_medio<=8)
    }//else if(color_pulsado == 1)
    else if(color_pulsado == 2 && tareas.f4 == ON){
      tareas.f4 = OFF;
      while(cont_medio<=8){
        P1OUT_bit.P6 = LED_ON;
        P1OUT_bit.P0 = LED_OFF;
        P1OUT_bit.P5 = LED_OFF;
        if(cont_medio == 8){
          P1OUT_bit.P6 = LED_OFF;
        }//if(cont_media == 8)
      }//while(cont_medio<=8)
    }//else if(color_leido == 2)
    dir_lectura++;
    cont_medio = 0;
  }// while((dir_lectura < dir_escritura)&& (has_perdido==0))
} // void pulsa_secuencia()

/* ====== RUTINAS TRATAMIENTO INTERRUPCION ================================== */

#pragma vector = TIMER0_A0_VECTOR
__interrupt void RTI_T0_TACCR0(void){
  cont_inicio++;
  cont_medio++;
  cont_final++;
}// __interrupt void RTI_T0_TACCR0

#pragma vector = WDT_VECTOR     //Antirrebote del pulsador S2, S3 y S4
__interrupt void RTI_WD(void){
  WDTCTL = WDTPW + WDTHOLD;  // Detiene watchdog
  P1IFG_bit.P3 = OFF;       //Limpia flag interrupcion P1.3
  P1IE_bit.P3 = ON;         //Habilita interrupcion P1.3
  P2IFG &=~(BIT0|BIT1);    //Limpia flag interrupcion P2.0 y P2.1
  P2IE |= (BIT0|BIT1);     //Habilita interrupcion P2.0 y P2.1
} // __interrupt void RTI_WD(void)

#pragma vector = PORT1_VECTOR
__interrupt void RTI_P1(void) {
  P1IE_bit.P3 = OFF;    //Deshabilita interrupcion P1.3
  
  // Antirrebote            
  WDTCTL = WDT_MDLY_32;    //Intervalo 32ms utilizando reloj de 1MHz
  IFG1_bit.WDTIFG = OFF;    //Limpia flag interrupcion watchdog
  IE1_bit.WDTIE = ON;      //Habilita interrupcion watchdog
  //Antirrebote
  
  if(P1IFG_bit.P3 == ON){
    if (P1IES_bit.P3 == ON){
      //Si flanco de bajada (pulsador presionado)
      tareas.f0 = ON;      //Activar Tarea 0
      //configura timer puerto 1
      TA0CTL_bit.TACLR = ON;  //Inicializa cuenta
      TA0CCTL0_bit.CCIE = ON;   //Habilita interrupcion Timer0.
      inicio = 1;
      cont_inicio = 0;
      if(tiempo == 1){ 
        inicio = 0;
        if(tareas.f0 == ON){     
          color_pulsado = 0;
        }// if(P1IES_bit.P3 == PULS_ON)
      tiempo = 0;
      cont_medio = 0;
      }// if(tiempo == 1)
    //configura timer puerto 1
    } // if (P1IES_bit.P3 == ON)
    else {
    //Si flanco de subida(pulsador soltado)
      tareas.f1 = ON;      //Activar Tarea_1
    }// else: if (P1IES_bit.P3 == ON)
    P1IES_bit.P3 = ~(P1IES_bit.P3);    //P1.3 -> Conmuta flanco
    P1IFG_bit.P3 = OFF;                // Limpia flag P1.3
    }//if(P1IFG_bit.P3 == ON)

  //Led apagados.
  P1OUT_bit.P0 = P1OUT_bit.P5 = P1OUT_bit.P6 = LED_OFF;
  
  if(tareas.total != OFF){
    //Si alguna tarea activa
    __low_power_mode_off_on_exit();  //Sale bajo consumo (LPM0)
  } // if (tareas.total != OFF)
} // void RTI_P1(void)

#pragma vector = PORT2_VECTOR
__interrupt void RTI_P2(void) {
  P2IE_bit.P0 = P2IE_bit.P1 = OFF;   //Deshabilita interrupcion P2.0 y P2.1
  
  //Antirrebote
  WDTCTL = WDT_MDLY_32;      //Intervalo de 32ms utilizando reloj de 1MHz
  IFG1_bit.WDTIFG = OFF;     //Limpia flag interrupcion watchdog
  IE1_bit.WDTIE = ON;      //Habilita interrupcion watchdog
  //Antirrebote

  if(P2IFG_bit.P0 == ON){
    if(P2IES_bit.P0 == ON){
      //Si flanco de bajada (pulsador presionado), nos da el dato de S3 
      tareas.f2 = ON;   // Activar Tarea_2
      //Configura Timer puerto 2
      TA0CTL_bit.TACLR = ON;  //Inicializa cuenta
      TA0CCTL0_bit.CCIE = ON;   //Habilita interrupcion Timer0.
      inicio = 1;
      cont_inicio = 0;
      if(tiempo == 1){
        inicio = 0;
        if(tareas.f2 == ON){
          color_pulsado = 1;
        }//if(tareas.f2 == ON)
        tiempo = 0;
        cont_medio = 0;
      }//if(tiempo == 1)
    }// if (P2IES_bit.P0 == ON)
    else{
      tareas.f3 = ON;
    }
    P2IES_bit.P0 = ~(P2IES_bit.P0); //P2.0 -> Conmuta flanco
    P2IFG_bit.P0 = OFF;             //Limpia flag P2.0
  }//if(P2IFG_bit.P0 == ON)
  
  if(P2IFG_bit.P1 == ON){
    if(P2IES_bit.P1 == ON){
      tareas.f4 = ON;   //Activar Tarea_4
      //Configura Timer puerto 2
      TA0CTL_bit.TACLR = ON;   //Inicializa cuenta
      TA0CCTL0_bit.CCIE = ON;  //Habilita interrupcion Timer0
      inicio = 1;
      cont_inicio = 0;
      if(tiempo == 1){
        inicio = 0;
        if(tareas.f4 == ON){
          color_pulsado = 2;
        }//if(tareas.f4 == ON)
        tiempo = 0;
        cont_medio = 0;
      }//if(tiempo == 1)
    }//if(P2IES_bit.P1 == ON)
    else{
      //Si flanco de subida(pulsador soltado)
      tareas.f5 = ON;  // Activar Tarea_5
    }// else: if(P2IES_bit.P1 == ON)
    P2IES_bit.P1 = ~(P2IES_bit.P1);  //P2.1 -> Conmuta flanco
    P2IFG_bit.P1 = OFF;              //Limpia flag P2.
  }//if(P2IFG_bit.P1 == ON)

  //Led apagados
  P1OUT_bit.P0 = P1OUT_bit.P5 = P1OUT_bit.P6 = LED_OFF;

  if(tareas.total != OFF){
    //Si alguna tarea activa
    __low_power_mode_off_on_exit();   //Sale bajo consumo (LPM0)
  } // if(tareas.total != OFF)  
} // void RTI_P2(void) 