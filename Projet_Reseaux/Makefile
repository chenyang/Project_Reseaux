#=============================================================================
#PROJET						:	Projet de réseaux
#NOM DU FICHIER		:	makefile
#OBJET						:	Ce fichier
#										- compile le projet complet
#										- supprime les fichiers intermediaires 
#										- force la régénération intégrale
#--------------------------------------------------------------------------------
#DATE DE CREATION	: 17/02/2012
#AUTEUR						: Marc Billy
#MODIFICATIONS	( n)	date					auteur					description
#
#								( 0)	17/02/2012		Marc BILLY 			Création...
#
#============================================================================= 



CC=gcc #désigne le compilateur utilisé: gcc
CFLAGS=-Wall #regroupe les options de compilation 
LDFLAGS=#regroupe les options de l'édition de liens

projetc : client serveur


client : ./func.o ./client.o
	$(CC) -o client ./func.o ./client.o -lm
	chmod +x client


serveur : ./func.o ./serveur.o
	$(CC) -o serveur ./func.o ./serveur.o -lm
	chmod +x serveur


./func.o : ./func.c
	$(CC) -o ./func.o -c ./func.c $(CFLAGS)


./serveur.o : ./serveur.c ./defs.h
	$(CC) -o ./serveur.o -c ./serveur.c $(CFLAGS)


./client.o : ./client.c ./defs.h
	$(CC) -o ./client.o -c ./client.c $(CFLAGS)
	

clean:
	rm  ./*.o
	rm ./client
	rm ./serveur
	
