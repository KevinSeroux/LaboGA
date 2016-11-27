#include <windows.h>
#include <random>
#include <bitset>
#include "Entete.h"
#pragma comment (lib,"GeneticDLL.lib")
//%%%%%%%%%%%%%%%%%%%%%%%%% IMPORTANT: %%%%%%%%%%%%%%%%%%%%%%%%% 
//Le fichier de probleme (.txt) et les fichiers de la DLL (GeneticDLL.dll et GeneticDLL.lib) doivent 
//se trouver dans le répertoire courant du projet pour une exécution à l'aide du compilateur. Indiquer les
//arguments du programme dans les propriétés du projet - débogage - arguements.
//Sinon, utiliser le répertoire execution.

//*****************************************************************************************
// Prototype des fonctions se trouvant dans la DLL 
//*****************************************************************************************
//DESCRIPTION:	Lecture du Fichier probleme et initialiation de la structure Problem
extern "C" _declspec(dllimport) void LectureProbleme(std::string FileName, TProblem & unProb, TGenetic &unGenetic);

//DESCRIPTION:	Fonction d'affichage à l'écran permettant de voir si les données du fichier problème ont été lues correctement
extern "C" _declspec(dllimport) void AfficherProbleme (TProblem unProb);

//DESCRIPTION:	Évaluation de la fonction objectif d'une solution et MAJ du compteur d'évaluation. 
//				Retourne un long représentant la valeur totale des objets sélectionnés
extern "C" _declspec(dllimport) void EvaluerSolution(TIndividu & uneSol, TProblem unProb, TGenetic &unGenetic);

//DESCRIPTION: Fonction qui vérifie si une solution respecte l'ensemble des contraintes. Retourne VRAI si c'est le cas, FAUX sinon.
extern "C" _declspec(dllimport) bool EstValide(TIndividu uneSol, TProblem unProb);

//DESCRIPTION: Fonction qui génére une population initiale en s'assurant d'avoir des solutions valides*/
extern "C" _declspec(dllimport) void CreerPopInitialeAleaValide(std::vector<TIndividu> & unePop, TProblem unProb, TGenetic & unGenetic);

//DESCRIPTION: Fonction qui affiche le détail des solutions (de Debut jusqu'a Fin-1) dans la population courante
extern "C" _declspec(dllimport) void AfficherSolutions(std::vector<TIndividu> unePop, int Debut, int Fin, int Iter, TProblem unProb);

//DESCRIPTION: Fonction de tri Quicksort des individus dans la population entre Debut et Fin-1 INCLUSIVEMENT 
extern "C" _declspec(dllimport) void TrierPopulation(std::vector<TIndividu> & unePop, int Debut, int Fin);

//DESCRIPTION: Copie de la séquence et de la fonction objectif dans une nouvelle TSolution. La nouvelle TSolution est retournée.
extern "C" _declspec(dllimport) void CopierSolution (const TIndividu uneSol, TIndividu &Copie, TProblem unProb);

//DESCRIPTION: Fonction qui réalise la MUTATION (modification aléatoire) sur une solution: Sélection aléatoire d'un objet et inversion du booléen.
//La solution est ensuite évaluée et vérifiée. Si elle ne respecte pas l'ensemble des contraintes du problème, sa fonction obj est pénalisée de 1M.
extern "C" _declspec(dllimport) void Mutation(TIndividu & Mutant, TProblem unProb, TGenetic & unGen);

//DESCRIPTION: Fonction de sélection d'un individu par tournoi
extern "C" _declspec(dllexport) int Selection (std::vector<TIndividu> unePop, int _Taille, TProblem unProb);

//DESCRIPTION: Fonction affichant les résultats de l'algorithme génétique
extern "C" _declspec(dllexport) void AfficherResultats (TIndividu uneBest, TProblem unProb, TGenetic unGen);

//DESCRIPTION: Fonction affichant les résultats de l'algorithme génétique dans un fichier texte
extern "C" _declspec(dllexport) void AfficherResultatsFichier (TIndividu uneBest, TProblem unProb, TGenetic unGen, std::string FileName);

//DESCRIPTION:	Libération de la mémoire allouée dynamiquement
extern "C" _declspec(dllexport) void LibererMemoireFinPgm (std::vector<TIndividu> & unePop, std::vector<TIndividu> & unePopEnfant, TIndividu & uneBest, TProblem & unProb, TGenetic unGen);

//*****************************************************************************************
// Prototype des fonctions locales 
//*****************************************************************************************
TIndividu Croisement(TIndividu Parent1, TIndividu Parent2, TProblem unProb, TGenetic & unGen);
void Remplacement(std::vector<TIndividu> & Parents, std::vector<TIndividu> Enfants, TProblem unProb, TGenetic unGen);

void MonIgnition(char *Param[]);
TIndividu MonAlgoGenetique(vector<TIndividu>& Pop, vector<TIndividu>& PopEnfant, const TProblem& unProb, TGenetic& unGen);
void MaPopulationInitiale(vector<TIndividu>& Pop, TIndividu& Best, const TProblem& unProb, TGenetic& unGen);
void MaSelectionCroisement(vector<TIndividu>& Pop, vector<TIndividu>& PopEnfant, const TProblem& unProb, TGenetic& unGen, vector<bool>& forcerMutation);
void MonCroisementUniforme(const TIndividu& Parent1, const TIndividu& Parent2, TIndividu& Enfant, const TProblem& unProb);
void MonCroisementPoint(const TIndividu& Parent1, const TIndividu& Parent2, TIndividu& Enfant, const TProblem& unProb);
void MonCroisement2Points(const TIndividu& Parent1, const TIndividu& Parent2, TIndividu& Enfant, const TProblem& unProb);
void MaReparation(vector<TIndividu>& PopEnfant, const TProblem& unProb, TGenetic& unGen);
void MaMutation(vector<TIndividu>& PopEnfant, const TProblem& unProb, TGenetic& unGen, const vector<bool>& forcerMutation);

//*****************************************************************************************
// Options pour la compilation :
// - Un seul croisement doit être #define, le reste #undef
//*****************************************************************************************
#define CROISEMENT_UNIFORME
#undef CROISEMENT_POINT
#undef CROISEMENT_2_POINTS
#undef VERBOSE

//*****************************************************************************************
// Variables globale : oui c'est sale mais :
// - On ne peut pas modifier les paramètres en entrée des fonctions pour gagner en performance
// - Dans un programme petit comme celui-ci, le problème de la maintenabilité n'est pas valide
//*****************************************************************************************
// Pour empêcher la dé/allocation sur le tas, et éviter de modifier les paramètres en entrée
// de la fonction de croisement
uint64_t* genesAleatoire = 0;

//******************************************************************************************
// Fonction main
//*****************************************************************************************
int main(int NbParam, char *Param[])
{
	MonIgnition(Param);

	system("PAUSE");
	return 0;
}

inline void MonIgnition(char *Param[])
{
	TProblem LeProb;					//**Définition de l'instance de problème
	TGenetic LeGenetic;					//**Définition des paramètres du recuit simulé
	std::vector<TIndividu> Pop;			//**Population composée de Taille_Pop Individus 
	std::vector<TIndividu> PopEnfant;	//**Population d'enfant

	string NomFichier;

	//**Lecture des paramètres
	NomFichier.assign(Param[1]);
	LeGenetic.TaillePop = atoi(Param[2]);
	LeGenetic.ProbCr = atof(Param[3]);
	LeGenetic.ProbMut = atof(Param[4]);
	LeGenetic.NB_EVAL_MAX = atoi(Param[5]);
	LeGenetic.TaillePopEnfant = (int)ceil(LeGenetic.ProbCr * LeGenetic.TaillePop);
	LeGenetic.Gen = 0;

	//**Définition de la dimension des tableaux
	Pop.resize(LeGenetic.TaillePop);				    //**Le tableau utilise les indices de 0 à TaillePop-1.
	PopEnfant.resize(LeGenetic.TaillePopEnfant);	    //**Le tableau utilise les indices de 0 à TaillePopEnfant-1
	genesAleatoire = new uint64_t[LeGenetic.TaillePop]; //**Le tableau utilise les indices de 0 à TaillePop-1.
	
	LectureProbleme(NomFichier, LeProb, LeGenetic);     //**Lecture du fichier de donnees

	//**MAIN LOOP
	TIndividu Best = MonAlgoGenetique(Pop, PopEnfant, LeProb, LeGenetic);

	AfficherResultats(Best, LeProb, LeGenetic);		//**NE PAS ENLEVER
	AfficherResultatsFichier(Best, LeProb, LeGenetic, "Resultats.txt");
	AfficherSolutions(vector<TIndividu>(1, Best), 0, 1, LeGenetic.Gen, LeProb);

	delete[] genesAleatoire;
	LibererMemoireFinPgm(Pop, PopEnfant, Best, LeProb, LeGenetic);
}

inline TIndividu MonAlgoGenetique(vector<TIndividu>& Pop, vector<TIndividu>& PopEnfant,
	const TProblem& unProb, TGenetic& unGen)
{
	TIndividu Best;	//**Meilleure solution depuis le début de l'algorithme
	vector<bool> forcerMutation(unGen.TaillePopEnfant, false);

	MaPopulationInitiale(Pop, Best, unProb, unGen);
	AfficherProbleme(unProb);

	//**Boucle principale de l'algorithme génétique
	do
	{
		unGen.Gen++;

		//**SELECTION et CROISEMENT
		MaSelectionCroisement(Pop, PopEnfant, unProb, unGen, forcerMutation);
		//AfficherSolutions(PopEnfant, 0, LeGenetic.TaillePopEnfant, LeGenetic.Gen, LeProb);

		//**MUTATION d'une solution
		MaMutation(PopEnfant, unProb, unGen, forcerMutation);
		//AfficherSolutions(PopEnfant, Sol, Sol, LeGenetic.Gen, LeProb);

		//**REPARATION operator
		MaReparation(PopEnfant, unProb, unGen);
		//AfficherSolutions(PopEnfant, Sol, Sol, LeGenetic.Gen, LeProb);

		//**REMPLACEMENT de la population pour la prochaine génération
		Remplacement(Pop, PopEnfant, unProb, unGen);
		//AfficherSolutions(Pop, 0, Sol, LeGenetic.Gen, LeProb);

		//**Conservation de la meilleure solution
		TrierPopulation(Pop, 0, unGen.TaillePop);
		if (Best.FctObj < Pop[0].FctObj)				//**NE PAS ENLEVER
		{
			CopierSolution(Pop[0], Best, unProb);
			cout << "Meilleure solution trouvee (Generation# " << unGen.Gen << "): " << Best.FctObj << endl;
		}

	} while (unGen.CptEval < unGen.NB_EVAL_MAX);	//**NE PAS ENLEVER

	return Best;
}

inline void MaPopulationInitiale(vector<TIndividu>& Pop, TIndividu& Best,
	const TProblem& unProb, TGenetic& unGen)
{
	//**Initialisation de la population initiale NB: Initialisation de la population entraine des evaluation de solutions.
	//**CptEval est donc = TaillePop au retour de la fonction.
	CreerPopInitialeAleaValide(Pop, unProb, unGen);

	//**Tri de la population
	TrierPopulation(Pop, 0, unGen.TaillePop);
#ifdef VERBOSE
	AfficherSolutions(Pop, 0, unGen.TaillePop, unGen.Gen, unProb);
#endif

	//**Initialisation de de la meilleure solution
	CopierSolution(Pop[0], Best, unProb);
	cout << endl << "Meilleure solution de la population initiale: " << Best.FctObj << endl << endl;  //**NE PAS ENLEVER
}

inline void MaSelectionCroisement(vector<TIndividu>& Pop,
	vector<TIndividu>& PopEnfant, const TProblem& unProb, TGenetic& unGen,
	vector<bool>& forcerMutation)
{
	for (int i = 0; i < unGen.TaillePopEnfant; i++)
	{
		//**SÉLECTION de deux parents
		int Pere = Selection(Pop, unGen.TaillePop, unProb);

		/* Pour empêcher que le père se reproduise avec lui-même, on met
		la valeur de sa fonction objectif à 0 pour la restaurer plus tard */
		long tempFctObjPere = Pop[Pere].FctObj;
		Pop[Pere].FctObj = 0;

		int Mere = Selection(Pop, unGen.TaillePop, unProb);

		// On restore la valeur de la fonction objectif du père
		Pop[Pere].FctObj = tempFctObjPere;

		//**CROISEMENT entre les deux parents. Création d'UN enfant.
		PopEnfant[i] = Croisement(Pop[Pere], Pop[Mere], unProb, unGen);

		//Si l'enfant ressemble trop au père, on force la mutation
		if (PopEnfant[i].Selec == Pop[Pere].Selec)
			forcerMutation[i] = true;


#ifdef VERBOSE
		cout << endl << "Pere " << Pere << " et Mere " << Mere << " donnent :" << endl;
		AfficherSolutions(PopEnfant, i, i + 1, unGen.Gen, unProb);
#endif
	}
}

//******************************************************************************************************
//**Fonction qui réalise le CROISEMENT (échange de genes) entre deux parents. Retourne l'enfant produit.
//******************************************************************************************************
//**A DÉFINIR PAR L'ÉTUDIANT****************************************************************************
//**NB: IL FAUT RESPECTER LA DEFINITION DES PARAMÈTRES AINSI QUE LE RETOUR DE LA FONCTION
//****************************************************************************************************** 
TIndividu Croisement(TIndividu Parent1, TIndividu Parent2, TProblem unProb, TGenetic & unGen)
{
	TIndividu Enfant;
	Enfant.Selec.resize(unProb.NbObj);

#ifdef CROISEMENT_UNIFORME
	MonCroisementUniforme(Parent1, Parent2, Enfant, unProb);
#endif

#ifdef CROISEMENT_POINT
	MonCroisementPoint(Parent1, Parent2, Enfant, unProb);
#endif

#ifdef CROISEMENT_2_POINTS
	MonCroisement2Points(Parent1, Parent2, Enfant, unProb);
#endif

	//**NE PAS ENLEVER
	return (Enfant);
}

inline void MaMutation(vector<TIndividu>& PopEnfant, const TProblem& unProb,
	TGenetic& unGen, const vector<bool>& forcerMutation)
{
	for (int i = 0; i < PopEnfant.size(); i++)
		if (forcerMutation[i] == true)
			Mutation(PopEnfant[i], unProb, unGen);

	for (int i = 0; i< ceil(unGen.ProbMut * unGen.TaillePopEnfant); i++)
	{
		//**Choix Aléatoire d'un enfant a muter
		int Sol = rand() % unGen.TaillePopEnfant;

		//**Pas besoin de muter si la mutation a été forcée
		if (forcerMutation[Sol] == false)
		{
			//AfficherSolutions(PopEnfant, Sol, Sol, LeGenetic.Gen, LeProb);
			Mutation(PopEnfant[Sol], unProb, unGen);
		}
	}
}

inline void MaReparation(vector<TIndividu>& PopEnfant, const TProblem& unProb,
	TGenetic& unGen)
{
	for (int i = 0; i < unGen.TaillePopEnfant; ++i)
	{
		EvaluerSolution(PopEnfant[i], unProb, unGen);

		if (!EstValide(PopEnfant[i], unProb))
		{

#ifdef VERBOSE
			cout << endl << "Avant reparation :" << endl;
			AfficherSolutions(PopEnfant, i, i + 1, unGen.Gen, unProb);
#endif
			do
			{
				int indexGeneAPermuter = rand() % unProb.NbObj;
				if (PopEnfant[i].Selec[indexGeneAPermuter] == true)
					PopEnfant[i].Selec[indexGeneAPermuter] = false;

			} while (!EstValide(PopEnfant[i], unProb));

			EvaluerSolution(PopEnfant[i], unProb, unGen);

#ifdef VERBOSE
			cout << endl << "Apres reparation :" << endl;
			AfficherSolutions(PopEnfant, i, i + 1, unGen.Gen, unProb);
#endif
		}
	}
}

inline void MaGenerationGenes(uint64_t* genes, int taillePop)
{
	/* Le premier appel est plus long mais à cause des initialisations
	ci-dessous. Mais parmis tous les autres appels c'est insignifiant */
	static std::random_device rd;
	static std::mt19937_64 e2(rd());
	static uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

	for (int i = 0; i < taillePop; i += 64)
		genes[i] = dis(e2);
}

inline void MonCroisementUniforme(const TIndividu& Parent1, const TIndividu& Parent2,
	TIndividu& Enfant, const TProblem& unProb)
{
	MaGenerationGenes(genesAleatoire, unProb.NbObj);

	for (int i = 0; i < unProb.NbObj; ++i)
	{
		// C'est 0 donc parent 1
		if ((genesAleatoire[i / 64] << (i % 64)) < 0x8000000000000000)
			Enfant.Selec[i] = Parent1.Selec[i];
		else // C'est 1 donc parent 2
			Enfant.Selec[i] = Parent2.Selec[i];
	}
}

inline void MonCroisementPoint(const TIndividu& Parent1, const TIndividu& Parent2,
	TIndividu& Enfant, const TProblem& unProb)
{
	//**INDICE: Le sous-programme rand() génère aléatoirement un nombre entier entre 0 et RAND_MAX (i.e., 32767) inclusivement.
	//**Pour tirer un nombre aléatoire entier entre 0 et MAX-1 inclusivement, il suffit d'utiliser l'instruction suivante : NombreAleatoire = rand() % MAX;

	int indexSeparation = rand() % unProb.NbObj;

	int i = 0;
	for(; i < indexSeparation; ++i)
		Enfant.Selec[i] = Parent1.Selec[i];
	for (; i < unProb.NbObj; ++i)
		Enfant.Selec[i] = Parent2.Selec[i];
}

inline void MonCroisement2Points(const TIndividu& Parent1, const TIndividu& Parent2,
	TIndividu& Enfant, const TProblem& unProb)
{
	//**INDICE: Le sous-programme rand() génère aléatoirement un nombre entier entre 0 et RAND_MAX (i.e., 32767) inclusivement.
	//**Pour tirer un nombre aléatoire entier entre 0 et MAX-1 inclusivement, il suffit d'utiliser l'instruction suivante : NombreAleatoire = rand() % MAX;

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

//*******************************************************************************************************
//Fonction qui réalise le REMPLACEMENT de la population pour la prochaine génération. Cette fonction doit
//prendre les TaillePop solutions de la population "Parents" et les TaillePopEnfant solutions de la 
//population "Enfants" et retenir SEULEMENT TaillePop solutions pour commencer la prochaine génération. 
//Les TaillePop solutions retenues doivent être placées dans la populations "Parents".  
//*******************************************************************************************************
//**A DÉFINIR PAR L'ÉTUDIANT*****************************************************************************
//**NB: IL FAUT RESPECTER LA DEFINITION DES PARAMÈTRES
//******************************************************************************************************* 
void Remplacement(std::vector<TIndividu> & Parents, std::vector<TIndividu> Enfants, TProblem unProb, TGenetic unGen)
{
	// POUR commencer on remplace les pires parents par les enfants
	for (size_t i = 0; i < Enfants.size(); ++i)
	{
		Parents[Parents.size() - i - 1] = Enfants[i];
	}
}
