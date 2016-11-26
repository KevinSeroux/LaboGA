#include <windows.h>
#include <random>
#include <bitset>
#include "Entete.h"
#pragma comment (lib,"GeneticDLL.lib")
//%%%%%%%%%%%%%%%%%%%%%%%%% IMPORTANT: %%%%%%%%%%%%%%%%%%%%%%%%% 
//Le fichier de probleme (.txt) et les fichiers de la DLL (GeneticDLL.dll et GeneticDLL.lib) doivent 
//se trouver dans le r�pertoire courant du projet pour une ex�cution � l'aide du compilateur. Indiquer les
//arguments du programme dans les propri�t�s du projet - d�bogage - arguements.
//Sinon, utiliser le r�pertoire execution.

//*****************************************************************************************
// Prototype des fonctions se trouvant dans la DLL 
//*****************************************************************************************
//DESCRIPTION:	Lecture du Fichier probleme et initialiation de la structure Problem
extern "C" _declspec(dllimport) void LectureProbleme(std::string FileName, TProblem & unProb, TGenetic &unGenetic);

//DESCRIPTION:	Fonction d'affichage � l'�cran permettant de voir si les donn�es du fichier probl�me ont �t� lues correctement
extern "C" _declspec(dllimport) void AfficherProbleme (TProblem unProb);

//DESCRIPTION:	�valuation de la fonction objectif d'une solution et MAJ du compteur d'�valuation. 
//				Retourne un long repr�sentant la valeur totale des objets s�lectionn�s
extern "C" _declspec(dllimport) void EvaluerSolution(TIndividu & uneSol, TProblem unProb, TGenetic &unGenetic);

//DESCRIPTION: Fonction qui v�rifie si une solution respecte l'ensemble des contraintes. Retourne VRAI si c'est le cas, FAUX sinon.
extern "C" _declspec(dllimport) bool EstValide(TIndividu uneSol, TProblem unProb);

//DESCRIPTION: Fonction qui g�n�re une population initiale en s'assurant d'avoir des solutions valides*/
extern "C" _declspec(dllimport) void CreerPopInitialeAleaValide(std::vector<TIndividu> & unePop, TProblem unProb, TGenetic & unGenetic);

//DESCRIPTION: Fonction qui affiche le d�tail des solutions (de Debut jusqu'a Fin-1) dans la population courante
extern "C" _declspec(dllimport) void AfficherSolutions(std::vector<TIndividu> unePop, int Debut, int Fin, int Iter, TProblem unProb);

//DESCRIPTION: Fonction de tri Quicksort des individus dans la population entre Debut et Fin-1 INCLUSIVEMENT 
extern "C" _declspec(dllimport) void TrierPopulation(std::vector<TIndividu> & unePop, int Debut, int Fin);

//DESCRIPTION: Copie de la s�quence et de la fonction objectif dans une nouvelle TSolution. La nouvelle TSolution est retourn�e.
extern "C" _declspec(dllimport) void CopierSolution (const TIndividu uneSol, TIndividu &Copie, TProblem unProb);

//DESCRIPTION: Fonction qui r�alise la MUTATION (modification al�atoire) sur une solution: S�lection al�atoire d'un objet et inversion du bool�en.
//La solution est ensuite �valu�e et v�rifi�e. Si elle ne respecte pas l'ensemble des contraintes du probl�me, sa fonction obj est p�nalis�e de 1M.
extern "C" _declspec(dllimport) void Mutation(TIndividu & Mutant, TProblem unProb, TGenetic & unGen);

//DESCRIPTION: Fonction de s�lection d'un individu par tournoi
extern "C" _declspec(dllexport) int Selection (std::vector<TIndividu> unePop, int _Taille, TProblem unProb);

//DESCRIPTION: Fonction affichant les r�sultats de l'algorithme g�n�tique
extern "C" _declspec(dllexport) void AfficherResultats (TIndividu uneBest, TProblem unProb, TGenetic unGen);

//DESCRIPTION: Fonction affichant les r�sultats de l'algorithme g�n�tique dans un fichier texte
extern "C" _declspec(dllexport) void AfficherResultatsFichier (TIndividu uneBest, TProblem unProb, TGenetic unGen, std::string FileName);

//DESCRIPTION:	Lib�ration de la m�moire allou�e dynamiquement
extern "C" _declspec(dllexport) void LibererMemoireFinPgm (std::vector<TIndividu> & unePop, std::vector<TIndividu> & unePopEnfant, TIndividu & uneBest, TProblem & unProb, TGenetic unGen);

//*****************************************************************************************
// Prototype des fonctions locales 
//*****************************************************************************************
TIndividu Croisement(TIndividu Parent1, TIndividu Parent2, TProblem unProb, TGenetic & unGen);
void Remplacement(std::vector<TIndividu> & Parents, std::vector<TIndividu> Enfants, TProblem unProb, TGenetic unGen);

//*****************************************************************************************
// Options pour la compilation :
// - Un seul croisement doit �tre #define, le reste #undef
//*****************************************************************************************
#define CROISEMENT_UNIFORME
#undef CROISEMENT_POINT
#undef CROISEMENT_2_POINTS

//*****************************************************************************************
// Variables globale : oui c'est sale mais :
// - On ne peut pas modifier les param�tres en entr�e des fonctions pour gagner en performance
// - Dans un programme petit comme celui-ci, le probl�me de la maintenabilit� n'est pas valide
//*****************************************************************************************
// Pour emp�cher la d�/allocation sur le tas, et �viter de modifier les param�tres en entr�e
// de la fonction de croisement
uint64_t* genesAleatoire = 0;

//******************************************************************************************
// Fonction main
//*****************************************************************************************
int main(int NbParam, char *Param[])
{
	TProblem LeProb;					//**D�finition de l'instance de probl�me
	TGenetic LeGenetic;					//**D�finition des param�tres du recuit simul�
	std::vector<TIndividu> Pop;			//**Population compos�e de Taille_Pop Individus 
	std::vector<TIndividu> PopEnfant;	//**Population d'enfant
	TIndividu Best;						//**Meilleure solution depuis le d�but de l'algorithme
	
	int Pere, Mere;						//**Indices de solution des parents
	int Sol;							//**Indice de solution pour la mutation
	int i;
	
	string NomFichier;
	
	//**Lecture des param�tres
	NomFichier.assign(Param[1]);
	LeGenetic.TaillePop		= atoi(Param[2]);
	LeGenetic.ProbCr		= atof(Param[3]);
	LeGenetic.ProbMut		= atof(Param[4]);
	LeGenetic.NB_EVAL_MAX	= atoi(Param[5]);
	LeGenetic.TaillePopEnfant	= (int)ceil(LeGenetic.ProbCr * LeGenetic.TaillePop);
	LeGenetic.Gen = 0;

	//**D�finition de la dimension des tableaux
	Pop.resize(LeGenetic.TaillePop);				    //**Le tableau utilise les indices de 0 � TaillePop-1.
	PopEnfant.resize(LeGenetic.TaillePopEnfant);	    //**Le tableau utilise les indices de 0 � TaillePopEnfant-1
	genesAleatoire = new uint64_t[LeGenetic.TaillePop]; //**Le tableau utilise les indices de 0 � TaillePop-1.

	//**Lecture du fichier de donnees
	LectureProbleme(NomFichier, LeProb, LeGenetic);
	AfficherProbleme(LeProb);

	//**Initialisation de la population initiale NB: Initialisation de la population entraine des evaluation de solutions.
	//**CptEval est donc = TaillePop au retour de la fonction.
	CreerPopInitialeAleaValide(Pop, LeProb, LeGenetic);
	AfficherSolutions(Pop, 0, LeGenetic.TaillePop, LeGenetic.Gen, LeProb);
	
	//**Tri de la population
	TrierPopulation(Pop, 0, LeGenetic.TaillePop);
	AfficherSolutions(Pop, 0, LeGenetic.TaillePop, LeGenetic.Gen, LeProb);

	//**Initialisation de de la meilleure solution
	CopierSolution(Pop[0], Best, LeProb);
	cout << endl << "Meilleure solution de la population initiale: " << Best.FctObj << endl << endl;  //**NE PAS ENLEVER
	
	//**Boucle principale de l'algorithme g�n�tique
	do 
	{
		LeGenetic.Gen ++;
		//**S�lection et croisement
		for (i=0; i<LeGenetic.TaillePopEnfant; i++)
		{
			//**S�LECTION de deux parents
			Pere = Selection(Pop, LeGenetic.TaillePop, LeProb);
			Mere = Selection(Pop, LeGenetic.TaillePop, LeProb);
			
			//**CROISEMENT entre les deux parents. Cr�ation d'UN enfant.
			PopEnfant[i] = Croisement(Pop[Pere], Pop[Mere], LeProb, LeGenetic);
		}
		AfficherSolutions(PopEnfant, 0, LeGenetic.TaillePopEnfant, LeGenetic.Gen, LeProb);
		
		//**MUTATION d'une solution
		for(i=0; i< ceil(LeGenetic.ProbMut*LeGenetic.TaillePopEnfant); i++)
		{
			//**Choix Al�atoire d'un enfant a muter
			Sol = rand() % LeGenetic.TaillePopEnfant; 
			AfficherSolutions(PopEnfant, Sol, Sol, LeGenetic.Gen, LeProb);
			Mutation(PopEnfant[Sol], LeProb, LeGenetic);
		}
		AfficherSolutions(PopEnfant, Sol, Sol, LeGenetic.Gen, LeProb);

		//**REMPLACEMENT de la population pour la prochaine g�n�ration
		Remplacement(Pop, PopEnfant, LeProb, LeGenetic);
		
		//**Conservation de la meilleure solution
		TrierPopulation(Pop, 0, LeGenetic.TaillePop);
		if (Best.FctObj < Pop[0].FctObj)				//**NE PAS ENLEVER
			CopierSolution(Pop[0], Best, LeProb);
		cout << "Meilleure solution trouvee (Generation# "<< LeGenetic.Gen << "): " << Best.FctObj << endl;

	}while (LeGenetic.CptEval < LeGenetic.NB_EVAL_MAX);	//**NE PAS ENLEVER

	AfficherResultats (Best, LeProb, LeGenetic);		//**NE PAS ENLEVER
	AfficherResultatsFichier (Best, LeProb, LeGenetic, "Resultats.txt");
	
	delete[] genesAleatoire;
	LibererMemoireFinPgm(Pop, PopEnfant, Best, LeProb, LeGenetic);
	system("PAUSE");
	return 0;
}

inline void GenererGenes(uint64_t* genes, int taillePop)
{
	/* Le premier appel est plus long mais � cause des initialisations
	ci-dessous. Mais parmis tous les autres appels c'est insignifiant */
	static std::random_device rd;
	static std::mt19937_64 e2(rd());
	static uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

	for (int i = 0; i < taillePop; i += 64)
		genes[i] = dis(e2);
}

inline void CroisementUniforme(const TIndividu& Parent1, const TIndividu& Parent2,
	TIndividu& Enfant, const TProblem& unProb)
{
	GenererGenes(genesAleatoire, unProb.NbObj);

	for (int i = 0; i < unProb.NbObj; ++i)
	{
		// C'est 0 donc parent 1
		if ((genesAleatoire[i / 64] << (i % 64)) < 0x8000000000000000)
			Enfant.Selec[i] = Parent1.Selec[i];
		else // C'est 1 donc parent 2
			Enfant.Selec[i] = Parent2.Selec[i];
	}
}

inline void CroisementPoint(const TIndividu& Parent1, const TIndividu& Parent2,
	TIndividu& Enfant, const TProblem& unProb)
{
	//**INDICE: Le sous-programme rand() g�n�re al�atoirement un nombre entier entre 0 et RAND_MAX (i.e., 32767) inclusivement.
	//**Pour tirer un nombre al�atoire entier entre 0 et MAX-1 inclusivement, il suffit d'utiliser l'instruction suivante : NombreAleatoire = rand() % MAX;

	int indexSeparation = rand() % unProb.NbObj;

	int i = 0;
	for(; i < indexSeparation; ++i)
		Enfant.Selec[i] = Parent1.Selec[i];
	for (; i < unProb.NbObj; ++i)
		Enfant.Selec[i] = Parent2.Selec[i];
}

inline void Croisement2Points(const TIndividu& Parent1, const TIndividu& Parent2,
	TIndividu& Enfant, const TProblem& unProb)
{
	//**INDICE: Le sous-programme rand() g�n�re al�atoirement un nombre entier entre 0 et RAND_MAX (i.e., 32767) inclusivement.
	//**Pour tirer un nombre al�atoire entier entre 0 et MAX-1 inclusivement, il suffit d'utiliser l'instruction suivante : NombreAleatoire = rand() % MAX;

	int milieu = unProb.NbObj / 2;
	int indexSeparation1 = rand() % milieu;
	int indexSeparation2 = milieu + (rand() % milieu);

	int i = 0;
	for (; i < indexSeparation1; ++i)
		Enfant.Selec[i] = Parent1.Selec[i];
	for (; i < indexSeparation2; ++i)
		Enfant.Selec[i] = Parent2.Selec[i];
	for (; i < unProb.NbObj; ++i)
		Enfant.Selec[i] = Parent1.Selec[i];
}

//******************************************************************************************************
//**Fonction qui r�alise le CROISEMENT (�change de genes) entre deux parents. Retourne l'enfant produit.
//******************************************************************************************************
//**A D�FINIR PAR L'�TUDIANT****************************************************************************
//**NB: IL FAUT RESPECTER LA DEFINITION DES PARAM�TRES AINSI QUE LE RETOUR DE LA FONCTION
//****************************************************************************************************** 
TIndividu Croisement(TIndividu Parent1, TIndividu Parent2, TProblem unProb, TGenetic & unGen)
{
	TIndividu Enfant;
	Enfant.Selec.resize(unProb.NbObj);

#ifdef CROISEMENT_UNIFORME
	CroisementUniforme(Parent1, Parent2, Enfant, unProb);
#elif CROISEMENT_POINT
	CroisementPoint(Parent1, Parent2, Enfant, unProb);
#elif CROISEMENT_2_POINTS
	Croisement2Points(Parent1, Parent2, Enfant, unProb);
#endif

	EvaluerSolution(Enfant, unProb, unGen);

	/*if (!EstValide(Enfant, unProb)) //Exemple appel � la fonction qui test la validit� d'une solution
	Penaliser la solution OU Corriger la solution OU ...*/

	//**NE PAS ENLEVER
	return (Enfant);
}

//*******************************************************************************************************
//Fonction qui r�alise le REMPLACEMENT de la population pour la prochaine g�n�ration. Cette fonction doit
//prendre les TaillePop solutions de la population "Parents" et les TaillePopEnfant solutions de la 
//population "Enfants" et retenir SEULEMENT TaillePop solutions pour commencer la prochaine g�n�ration. 
//Les TaillePop solutions retenues doivent �tre plac�es dans la populations "Parents".  
//*******************************************************************************************************
//**A D�FINIR PAR L'�TUDIANT*****************************************************************************
//**NB: IL FAUT RESPECTER LA DEFINITION DES PARAM�TRES
//******************************************************************************************************* 
void Remplacement(std::vector<TIndividu> & Parents, std::vector<TIndividu> Enfants, TProblem unProb, TGenetic unGen)
{
	//**D�claration et dimension dynamique d'une population temporaire pour contenir tous les parents et les enfants
	//std::vector<TIndividu> Temporaire;
	//Temporaire.resize(unGen.TaillePop + unGen.TaillePopEnfant);
	
	//**Pour trier toute la population temporaire, il suffit de faire l'appel suivant: TrierPopulation(Temporaire, 0, unGen.TaillePop+unGen.TaillePopEnfant);
	//**Pour copie une solution de Parents dans Temporaire, il suffit de faire l'appel suivant: CopierSolution(Parents[i], Temporaire[i], unProb);
	
	
	//METHODE BIDON: La population Parents demeure inchang�e
	
	
	//**Lib�ration de la population temporaire
	//for(i=0; i< unGen.TaillePop; i++)
	//	Temporaire[i].Selec.clear();
	//Temporaire.clear();
}
