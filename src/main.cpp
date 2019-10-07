#include <Arduino.h>

const int nb_ouverture = 3;
const int nb_flash     = 4;
const int nb_events    = nb_ouverture + nb_ouverture + nb_flash; //ouvertures + fermetures + flashs

const int triggerTypeOuverture = 1;
const int triggerTypeFermeture = 2;
const int triggerTypeFlash     = 3;

int ouverture_durees[nb_ouverture]; //Durée de chaque ouverture
int ouverture_delais[nb_ouverture]; //Delai de demarrage pour chacune des ouvertures
int flash_delais[nb_flash];         //Delai de demarrage pour chacun des flashs

int     amorce  = 500; //Duree de l'amorce en ms
boolean started = true;

int pin_V1 = 8;

void initPins() {
	pinMode(pin_V1, OUTPUT);
  	digitalWrite(pin_V1, LOW);
}

void initConfiguration() {
  
  ouverture_delais[0] = 0;
  ouverture_durees[0] = 50;

  ouverture_delais[1] = 150;
  ouverture_durees[1] = 50;

  ouverture_delais[2] = 500;
  ouverture_durees[2] = 50;

  flash_delais[0] = 2000;
  flash_delais[1] = 2000;

}

void doAmorce() {
  Serial.print("Amorce pour une durée de " );
  Serial.print( amorce );
  Serial.println( "ms" );
  digitalWrite(pin_V1, HIGH);
  delay( amorce );
  digitalWrite(pin_V1, LOW);
  delay(500);
}

void triggerOuvertureVanne( int indexOuverture ) {
  if ( ouverture_durees[indexOuverture] > 0 ) {
	digitalWrite(pin_V1, HIGH);
    Serial.print("Déclenchement de l'ouverture " );
    Serial.print( indexOuverture );
    Serial.print(" (delai : " );
    Serial.print( ouverture_delais[indexOuverture] ); 
    Serial.print("ms) pour " );
    Serial.print( ouverture_durees[indexOuverture] ); 
    Serial.println( "ms" );
  }
}

void triggerFermetureVanne( int indexFermeture ) {
  if ( ouverture_durees[indexFermeture] > 0 ) {
	digitalWrite(pin_V1, LOW);
    Serial.print("Déclenchement de la fermeture " );
    Serial.print( indexFermeture );
    Serial.print(" apres " );
    Serial.print( ouverture_durees[indexFermeture] ); 
    Serial.println( "ms" );
  }
}

void triggerFlash( int indexFlash ) {
  if ( flash_delais[indexFlash] > 0 ) {
    Serial.print("Déclenchement du flash " );
    Serial.print( indexFlash );
    Serial.print(" (delai : " );
    Serial.print( flash_delais[indexFlash] ); 
    Serial.println( "ms)" );
  }
}

void setup() {
  
  //Initialisation du Serial pour debug
Serial.begin(9600);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB
	}
  initPins();
  initConfiguration();
  doAmorce() ;
}

void loop() {
  
  if ( started ) {
  
  	unsigned long time = millis();
		int           triggerOrder[nb_events];
	  unsigned long triggerTime[nb_events];
		int           triggerTarget[nb_events];
		int           triggerTargetId[nb_events];
		
		//Initialisation des tableaux de travail
		for (int i=0; i<nb_events; i++) {
			triggerOrder[i]    = -1;
			triggerTime[i]     = -1;
			triggerTarget[i]   = -1;
			triggerTargetId[i] = -1;		
		}
		
		//Enregistrement des ouvertures de vanne
		for ( int i=0; i<nb_ouverture; i++ ) {
			//Ouverture de vanne
      triggerTime[i]     = ouverture_delais[i] + time;
			triggerTarget[i]   = triggerTypeOuverture;
			triggerTargetId[i] = i;
		}	

    //Enregistrement des fermetures de vanne
		for ( int i=0; i<nb_ouverture; i++ ) {
			//Ouverture de vanne
      triggerTime[nb_ouverture + i]     = ouverture_delais[i] + ouverture_durees[i] + time;
			triggerTarget[nb_ouverture + i]   = triggerTypeFermeture;
			triggerTargetId[nb_ouverture + i] = i;
		}	
		
		//Enregistrement des flashs
		for ( int i=0; i<nb_flash; i++ ) {
			triggerTime[nb_ouverture + nb_ouverture + i]     = flash_delais[i] + time;
			triggerTarget[nb_ouverture + nb_ouverture + i]   = triggerTypeFlash;	
			triggerTargetId[nb_ouverture + nb_ouverture + i] = i;
		}	
		
		//Determination de l'ordre d'execution des evenements
		for (  int i=0; i<nb_events; i++ ) {
			
			unsigned long min = 9999999;
			int minIndex = -1;
			
			for ( int j=0; j<nb_events; j++ ) {
				
				//On recherche d'abord si l'index courant a été traité
				bool isSorted = false;
				for (  int k=0; k<nb_events; k++ ) {
					if ( triggerOrder[k] == j ) isSorted = true;
				}
				
				//Si l'index courant n'a pas ete traite, on regarde si il est le plus petit
				if ( !isSorted && min > triggerTime[j] ) {
					min = triggerTime[j];
					minIndex = j;
				}
								
			}
			
			//On ajoute l'index courant a la liste des index traités
			triggerOrder[i] = minIndex;
			
		}
				
		//Declenchement des evenements dans l'ordre defini
		for ( int i=0; i<nb_events; i++ ) {
			
			int indexToExecute = triggerOrder[i];
			
			while ( millis() < triggerTime[indexToExecute] ) {
				;
			}
			
			if ( triggerTarget[indexToExecute] == triggerTypeOuverture ) {
				triggerOuvertureVanne( triggerTargetId[indexToExecute] );
      } else if ( triggerTarget[indexToExecute] == triggerTypeFermeture ) {
        triggerFermetureVanne( triggerTargetId[indexToExecute] );
			} else {
				triggerFlash( triggerTargetId[indexToExecute] );
			}
			
		}

    started = false;

  } 

}