#ifndef __ENTETE_H_
#define __ENTETE_H_

#include <cstdio>
#include <cstdlib> 
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>  
#include <cmath>
#include <vector>
using namespace std;


struct TProblem							//**Définition du problème:
{
	std::string Nom;					//**Nom du fichier de données
	int NbCt;							//**Nbre de CONTRAINTES indiquées dans le fichier
	int NbObj;							//**Nbre d'OBJETS indiqués dans le fichier
	std::vector<std::vector <int> > Poids;	//**POIDS pour de chaque objet pour chaque contrainte. NB: Tableaux de 0 à NbCt-1 et de 0 à NbObj-1. 
	std::vector<int> Capacite;			//**CAPACITÉ des sacs.  NB: Tableau de 0 à NbCt-1.
	std::vector<int> Valeur;			//**VALEUR de chaque objet	NB: Tableau de 0 à NbObj-1.
};

struct TIndividu						//**Définition d'une solution: 
{
	std::vector<bool> Selec;			//**Ensemble de gènes: indique si l'objet i est sélectionné (true) ou non (false). NB: Tableau de 0 à NbObj-1.
	long FctObj;						//**Valeur de la fonction objectif: Sommation de la valeur des objets sélectionnés
};

struct TGenetic
{
	int		TaillePop;					//**Taille de la population (nombre d'individus)
	int		TaillePopEnfant;			//**Taille de la populationEnfant (nombre d'enfants)
	double	ProbCr;						//**Probabilité de croisement [0%,100%]
	double	ProbMut;					//**Probabilité de mutation [0%,100%] 
	int		Gen;						//**Compteur du nombre de générations

	int		CptEval;					//**COMPTEUR DU NOMBRE DE SOLUTIONS EVALUEES. SERT POUR CRITERE D'ARRET.
	int		NB_EVAL_MAX;				//**CRITERE D'ARRET: MAXIMUM "NB_EVAL_MAX" EVALUATIONS.
};

#endif