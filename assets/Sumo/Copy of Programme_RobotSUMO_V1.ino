// Inclusion des fichier header des bibliothèques de fonctions Arduino
#include <arduino.h>
#include <IRremote.h>
#include <IRremoteInt.h>
// Définition des pins
#define Recepteur_Infrarouge          5
#define CapteurLigneAvantGauche_Pin  A2
#define CapteurLigneAvantDroite_Pin  A1
#define CapteurLigneArriere_Pin      A3
#define CapteurAvantGauche_echo       8
#define CapteurAvantDroite_echo      12
#define CapteurAvantGauche_trig       7
#define CapteurAvantDroite_trig      13
#define CapteurGauche_Pin            A5
#define CapteurDroite_Pin            A4
#define DigitalBatterie_Pin           4
#define AnalogueBatterie_Pin         A0
#define MOTG                          3
#define MOTD                         11
#define PWMG                          9
#define PWMD                         10
#define HIGH_MODE                     2
#define Buzzer_Pin                    6

// Définition des constantes

//Définition de variables global
#define Adresse_NEC_1     0xFF30CF // Trame associée au bouton START 
#define Adresse_NEC_2     0xFF18E7 // Trame associée au bouton ATTENTION
#define Adresse_NEC_3     0xFF7A85 // Trame associée au bouton STOP

char PosB;// Position du robot
bool Broken = false;// Variable permettant le fonctionnement du main (boucle while)
int LigneAvG; //Variable attestant la présence d'une ligne pour le capteur avant gauche
int LigneAvD; //Variable attestant la présence d'une ligne pour le capteur avant droite
int LigneAr; //Variable attestant la présence d'une ligne pour le capteur avant arrière
int UltrasonG; //Variable attestant la présence d'un robot pour le capteur ultrason gauche
int UltrasonD; //Variable attestant la présence d'un robot pour le capteur ultrason droite
int TelemetriqueG; //Variable attestant la présence d'un robot pour le capteur télémétrique gauche
int TelemetriqueD; //Variable attestant la présence d'un robot pour le capteur télémétrique droite
int ValeurNEC;
long duree; // Durée de l'écho
int distance; // Distance pour le capteur ultrason
const int PERIMETRE = 57; // Périmètre de détection (77 cm(taille doyo) - 2*10 cm(taille robots))
const int valeurDeReference = 600; // seuil pour noir et blanc trouvé par test
int Temps_n;
int Temps_n_1 = 0;
bool temps = true;

//Fonction du bloc Acquisition
int Capteur_Telemetrique(int n_capteur) {
  // Lecture du capteur télémétrique
  int valeurBrute = analogRead(n_capteur); 
  float tension = valeurBrute * (5.0 / 1023.0); // Calcul de la tension de la valeur récupérée par le capteur droit
  float distance = 13.0 * pow(tension, -1); // Estimation empirique de la distance entre le capteur télémétrique droit et l'adversaire
  if (distance <= PERIMETRE) {
    return 1;
  }
  return 0;
}

int Capteur_Ligne(int n_capteur) {
  int valeur = analogRead(n_capteur); // Lire la valeur analogique
    if (valeur < valeurDeReference){
      return 1; }
    else{
      return 0; }
}

int Capteur_Ultrason(int n_capteur_trig, int n_capteur_echo) {
  // Émission d'un signal de durée 10 microsecondes
  digitalWrite(n_capteur_trig, LOW);
  delayMicroseconds(5);
  digitalWrite(n_capteur_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(n_capteur_trig, LOW);

  // Écoute de l'écho
  duree = pulseIn(n_capteur_echo, HIGH);

  // Calcul de la distance
  distance = (duree * 0.034) / 2;
  if (distance <= PERIMETRE) {
    return 1;
  }
  return 0;


}

IRrecv irrecv(Recepteur_Infrarouge);  // associe la réception infrarouge à la broche " Recepteur_Infrarouge"
decode_results Valeur; // décoder la trame NEC reçu
int Lire_Trame_NEC(void) {
  int ValeurNEC = 0;
  if (irrecv.decode(&Valeur)) {
    switch (Valeur.value) { // si le récepteur reçoit une trame alors 3 choix possibles

      case  Adresse_NEC_1:  //Dans le cas où le bouton 1 (START) est appuyé
        ValeurNEC = 1;
        break;

      case  Adresse_NEC_2: //Dans le cas où le bouton 2 (ATTENTION) est appuyé
        ValeurNEC = 2;
        break;

      case  Adresse_NEC_3: //Dans le cas où le bouton 3 (STOP) est appuyé
        ValeurNEC = 3;
        break;

    }
    irrecv.resume(); // Préparer le récepteur pour le prochain signal

  }
  return ValeurNEC;
}

//Fonction du bloc Action
void Buzzer(void) {
  for(int Cycle=0;Cycle<50;Cycle++){
    digitalWrite(Buzzer_Pin, HIGH);
    delayMicroseconds(3333);
    digitalWrite(Buzzer_Pin, LOW);
    delayMicroseconds(3333);
  }
}

void Turn_itself(int sens, int angle) {
  temps = true;
  Temps_n_1 = 0;
  if (sens == 'G'){
    if (angle == 0){
      digitalWrite(MOTD,0);
      digitalWrite(MOTG,1);
      analogWrite(PWMD,255);
      analogWrite(PWMG,255);
    }
    else if (angle == 90){
      while (temps){
        Temps_n = millis();
        digitalWrite(MOTD,0);
        digitalWrite(MOTG,1);
        analogWrite(PWMD,255);
        analogWrite(PWMG,255);
        if ((Temps_n-Temps_n_1) > 224){
           Temps_n_1 = Temps_n;
           analogWrite(PWMD,0);
           analogWrite(PWMG,0);
           temps = false;
          }
        }
    }
    else if (angle == 135){
      while (temps){
        Temps_n = millis();
        digitalWrite(MOTD,0);
        digitalWrite(MOTG,1);
        analogWrite(PWMD,255);
        analogWrite(PWMG,255);
        if ((Temps_n-Temps_n_1) > 550){
           Temps_n_1 = Temps_n;
           analogWrite(PWMD,0);
           analogWrite(PWMG,0);
           temps = false;
        }
      }
   }
  
    else if (angle == 180){
     while (temps){
       Temps_n = millis();
       digitalWrite(MOTD,0);
       digitalWrite(MOTG,1);
       analogWrite(PWMD,255);
       analogWrite(PWMG,255);
       if ((Temps_n-Temps_n_1) > 1000){
         Temps_n_1 = Temps_n;
         analogWrite(PWMD,0);
         analogWrite(PWMG,0);
         temps = false;
       }
      }
     }
       }
  if (sens == 'D'){
    if (angle == 0){
      digitalWrite(MOTD,1);
      digitalWrite(MOTG,0);
      analogWrite(PWMD,255);
      analogWrite(PWMG,255);
    }
    else if (angle == 90){
         while (temps){
           Temps_n = millis();
           digitalWrite(MOTD,1);
           digitalWrite(MOTG,0);
           analogWrite(PWMD,255);
           analogWrite(PWMG,255);
           if ((Temps_n-Temps_n_1) > 224){
             Temps_n_1 = Temps_n;
             analogWrite(PWMD,0);
             analogWrite(PWMG,0);
             temps = false;
           }
          }
         }
    else if (angle == 135){
     while (temps){
       Temps_n = millis();
       digitalWrite(MOTD,1);
       digitalWrite(MOTG,0);
       analogWrite(PWMD,255);
       analogWrite(PWMG,255);
       if ((Temps_n-Temps_n_1) > 550){
         Temps_n_1 = Temps_n;
         analogWrite(PWMD,0);
         analogWrite(PWMG,0);
         temps = false;
       }
      }
     }     
    else if (angle == 180){
     while (temps){
       Temps_n = millis();
       digitalWrite(MOTD,1);
       digitalWrite(MOTG,0);
       analogWrite(PWMD,255);
       analogWrite(PWMG,255);
       if ((Temps_n-Temps_n_1) > 1000){
         Temps_n_1 = Temps_n;
         analogWrite(PWMD,0);
         analogWrite(PWMG,0);
         temps = false;
       }
      }
     }
   }
       
}

void Reculer(int vitesse) {
  digitalWrite(MOTD, 0);
  digitalWrite(MOTG, 0);
  analogWrite(PWMD, vitesse);
  analogWrite(PWMG, vitesse);
}

void Avancer(int vitesse) {
  digitalWrite(MOTD, 1);
  digitalWrite(MOTG, 1);
  analogWrite(PWMD, vitesse);
  analogWrite(PWMG, vitesse);
}

void Reorienter(int vitesse, char sens) {
  if (sens == 'D') {
    analogWrite(PWMG, vitesse * 0.50);
    analogWrite(PWMD, vitesse);
  }
  if (sens == 'G') {
    analogWrite(PWMD, vitesse * 0.50);
    analogWrite(PWMG, vitesse);
  }
}


// Fonction du bloc Energie
int Etat_batterie(uint8_t pin_lecture_info_batt) { // Lis la valeur en sortie du comparateur afin d'indiquer l'état de la batterie
  return digitalRead(pin_lecture_info_batt); // Retourne directement la valeur numérique avec 1 correspondant à une batterie inférieur à 6,7 V, et 0 correspondant à une batterie supérieur à 6,7 V
}

//Fonction du bloc Traitement
void Initialisation(void) {
  while ((PosB != 'G') || (PosB != 'D')) { //Boucle détectant la position du robot adverse au début du combat, soit à Gauche ('G') soit à Droite ('D') ou coupant la recherche en cas d'appui sur le bouton start de la télécommande
    if (Lire_Trame_NEC() == 1) {
      break;
    }
    else if (Capteur_Telemetrique(CapteurGauche_Pin) == 1) {
      PosB = 'G';
    }
    else if (Capteur_Telemetrique(CapteurDroite_Pin) == 1) {
      PosB = 'D';
    }
  }
  while (1) { //Boucle permettant au robot de démarer le combat(break) en cas d'appui sur le bouton start de la télécommande, ou de faire sonne le buzzer en cas d'appui sur le bouton attention de la télécommande
    if (Lire_Trame_NEC() == 1) {
      break;
    }
    else if (Lire_Trame_NEC() == 2) {
      Buzzer();
    }
  }
}

void Foncer(int PositionRobotAdv) { //Fonction permettant au robot de rapidement se tourner vers son adversaire et d'avancer vers lui au début du combat
  Turn_itself(PositionRobotAdv, 90);
  Avancer(255);
}
void Eviter_ligne(void) { // Fonction permettant au robot de s'éloigner d'une ligne sans tomber
  if ((LigneAvG == 1) && (LigneAvD == 1)) {
    Turn_itself('D', 180);
    Avancer(255);
  }
  else if (LigneAvG == 1) {
    Turn_itself('D', 135);
    Avancer(255);
    delay(500);
  }
  else if (LigneAvD == 1) {
    Turn_itself('G', 135);
    Avancer(255);
    delay(500);
  }
  else if (LigneAr == 1) {
    Avancer(255);
    delay(500);
  }
}

void Attaquer(void) {
  if ((UltrasonG == 1) && (UltrasonD == 1)) {
    Avancer(255);
  }
  else if (UltrasonG == 1) {
    Reorienter(255, 'G');
    PosB = 'G';
  }
  else if (UltrasonD == 1) {
    Reorienter(255, 'D');
    PosB = 'D';
  }
  else if (TelemetriqueG == 1) {
    Turn_itself('G', 0);
    PosB = 'G';
  }
  else if (TelemetriqueD == 1) {
    Turn_itself('D', 0);
    PosB = 'D';
  }
}

void Recherche(void) {
  Turn_itself(PosB, 0);
}

void setup() {
  // Configurer Timer1 pour une fréquence PWM d’environ 10 kHz
  TCCR1A = 0; // Réinitialiser le registre TCCR1A
  TCCR1B = 0; // Réinitialiser le registre TCCR1B
  TCCR1A |= (1 << WGM11); // Mode Fast PWM, TOP = ICR1
  TCCR1B |= (1 << WGM12) | (1 << WGM13); // Mode Fast PWM
  TCCR1B |= (1 << CS11); // Prescaler de 8

  // Calcul pour 10 kHz : F_CPU / (Prescaler * (1 + TOP))
  // 16 MHz / (8 * (1 + 199)) = 10 kHz
  ICR1 = 199; // TOP = 199 pour obtenir ~10 kHz

  pinMode(MOTD, OUTPUT);
  pinMode(MOTG, OUTPUT);
  pinMode(PWMD, OUTPUT);
  pinMode(PWMG, OUTPUT);
  pinMode(HIGH_MODE, OUTPUT);
  digitalWrite(HIGH_MODE, 1);
  pinMode(Recepteur_Infrarouge, INPUT); // Configure la broche infrarouge en entrée
  pinMode(CapteurAvantGauche_trig, OUTPUT); // Configuration du port Trigger gauche comme SORTIE
  pinMode(CapteurAvantGauche_echo, INPUT);  // Configuration du port Echo gauche comme ENTREE
  pinMode(CapteurAvantDroite_trig, OUTPUT); // Configuration du port Trigger droit comme SORTIE
  pinMode(CapteurAvantDroite_echo, INPUT);  // Configuration du port Echo droit comme ENTREE
  pinMode(CapteurGauche_Pin,INPUT);
  pinMode(CapteurDroite_Pin,INPUT);
  pinMode(CapteurLigneAvantGauche_Pin,INPUT);
  pinMode(CapteurLigneAvantDroite_Pin,INPUT);
  pinMode(CapteurLigneArriere_Pin,INPUT);
  pinMode(Buzzer_Pin,OUTPUT);
  
  irrecv.enableIRIn();  // Active la réception IR
  digitalWrite(MOTD, 1);
  digitalWrite(MOTG, 1);
  analogWrite(PWMD, 0);
  analogWrite(PWMG, 0);
  /*
  if(Etat_batterie(DigitalBatterie_Pin)==0){
    Initialisation();
    Foncer(PosB);
  }
  */
}

void loop() {
  Buzzer();
  delay(5000);
  /*
    while(Broken!= true){
    LigneAvG = Capteur_Ligne(CapteurLigneAvantGauche_Pin);
    LigneAvD = Capteur_Ligne(CapteurLigneAvantDroite_Pin);
    LigneAr = Capteur_Ligne(CapteurLigneArriere_Pin);
    UltrasonG = Capteur_Ultrason(CapteurAvantGauche_trig, CapteurAvantGauche_echo);
    UltrasonD = Capteur_Ultrason(CapteurAvantDroite_trig, CapteurAvantDroite_echo);
    TelemetriqueG = Capteur_Telemetrique(CapteurGauche_Pin);
    TelemetriqueD = Capteur_Telemetrique(CapteurDroite_Pin);
    if(Etat_batterie(DigitalBatterie_Pin)==1 || Lire_Trame_NEC()==3){
      Avancer(0);
      Broken = true;
      break;
    }
    else if((LigneAvG==1) || (LigneAvD==1) || (LigneAr==1)){
      Eviter_ligne();
    }
    else if((UltrasonG==1) || (UltrasonD==1) || (TelemetriqueG==1) || (TelemetriqueD==1)){
      Attaquer();
    }
    else{
      Recherche();
    }
    }
    */
}
