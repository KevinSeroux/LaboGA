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


struct TProblem							//**D�finition du probl�me:
{
	std::string Nom;					//**Nom du fichier de donn�es
	int NbCt;							//**Nbre de CONTRAINTES indiqu�es dans le fichier
	int NbObj;							//**Nbre d'OBJETS indiqu�s dans le fichier
	std::vector<std::vector <int> > Poids;	//**POIDS pour de chaque objet pour chaque contrainte. NB: Tableaux de 0 � NbCt-1 et de 0 � NbObj-1. 
	std::vector<int> Capacite;			//**CAPACIT� des sacs.  NB: Tableau de 0 � NbCt-1.
	std::vector<int> Valeur;			//**VALEUR de chaque objet	NB: Tableau de 0 � NbObj-1.
};

struct TIndividu						//**D�finition d'une solution: 
{
	std::vector<bool> Selec;			//**Ensemble de g�nes: indique si l'objet i est s�lectionn� (true) ou non (false). NB: Tableau de 0 � NbObj-1.
	long FctObj;						//**Valeur de la fonction objectif: Sommation de la valeur des objets s�lectionn�s
};

struct TGenetic
{
	int		TaillePop;					//**Taille de la population (nombre d'individus)
	int		TaillePopEnfant;			//**Taille de la populationEnfant (nombre d'enfants)
	double	ProbCr;						//**Probabilit� de croisement [0%,100%]
	double	ProbMut;					//**Probabilit� de mutation [0%,100%] 
	int		Gen;						//**Compteur du nombre de g�n�rations

	int		CptEval;					//**COMPTEUR DU NOMBRE DE SOLUTIONS EVALUEES. SERT POUR CRITERE D'ARRET.
	int		NB_EVAL_MAX;				//**CRITERE D'ARRET: MAXIMUM "NB_EVAL_MAX" EVALUATIONS.
};

#endif