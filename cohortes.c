#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define PI 3.14159265358979
#define AGE_MAX 180               // 15 ans en mois
#define MAT_MIN 5                 // maturite minimale (mois)
#define BEBES_MIN 3               // bebes minimum par portee
#define BEBES_MAX 6               // bebes maximum par portee
#define CIBLE 1000000000LL        // 1 milliard
#define DIX_MILLIONS 10000000LL   // pour comparer avec le modele individuel
#define MOIS_MAX 240              // 20 ans
#define NB_EXP 30                 // experiences par condition

// nombre de lapins par age (en mois)
long long fem[AGE_MAX + 1];
long long mal[AGE_MAX + 1];

long long naissances;             // naissances du dernier mois
long long morts;                  // morts du dernier mois
int moisDix = 0;                  // mois ou 10 millions sont atteints (0 si jamais)

// entier aleatoire entre a et b
int alea(int a, int b) {
    return a + rand() % (b - a + 1);
}

// reel aleatoire entre 0 et 1 (jamais 0)
double uniforme() {
    double a = (double)rand() * (RAND_MAX + 1.0) + rand() + 1.0;
    return a / ((RAND_MAX + 1.0) * (RAND_MAX + 1.0) + 1.0);
}

// loi normale (Box-Muller)
double normale() {
    return sqrt(-2.0 * log(uniforme())) * cos(2.0 * PI * uniforme());
}

// nombre de succes sur nb essais de probabilite p
long long binomiale(long long nb, double p) {
    long long i, k = 0;
    double moy, ecart;
    if (nb <= 0) return 0;
    if (nb <= 50) {                    // petit effectif : tirage exact
        for (i = 0; i < nb; i++) {
            if (uniforme() < p) k++;
        }
        return k;
    }
    moy = nb * p;                      // grand effectif : loi normale
    ecart = sqrt(nb * p * (1.0 - p));
    k = (long long)(moy + ecart * normale() + 0.5);
    if (k < 0) k = 0;
    if (k > nb) k = nb;
    return k;
}

// part des lapins matures a un age (maturite entre 5 et 8 mois)
double partMat(int age) {
    if (age < 5) return 0.0;
    if (age == 5) return 0.25;
    if (age == 6) return 0.50;
    if (age == 7) return 0.75;
    return 1.0;
}

// probabilite de survivre un mois a un age donne
double survieCohorte(int age) {
    double pj, adulte;
    if (age >= AGE_MAX) return 0.0;
    pj = 1.0 - partMat(age);           // part de jeunes
    if (age < 120) adulte = 0.60;
    else adulte = 0.60 * (15.0 - age / 12.0) / 5.0;
    return pj * pow(0.35, 1.0 / 12) + (1.0 - pj) * pow(adulte, 1.0 / 12);
}

long long totalFem() {
    long long t = 0;
    int a;
    for (a = 0; a <= AGE_MAX; a++) t += fem[a];
    return t;
}

long long totalMal() {
    long long t = 0;
    int a;
    for (a = 0; a <= AGE_MAX; a++) t += mal[a];
    return t;
}

long long total() {
    return totalFem() + totalMal();
}

// age moyen en mois
double ageMoyen() {
    double somme = 0.0;
    long long t = total();
    int a;
    if (t == 0) return 0.0;
    for (a = 0; a <= AGE_MAX; a++) {
        somme += (double)(fem[a] + mal[a]) * a;
    }
    return somme / t;
}

// remise a zero : les couples de depart sont des adultes de 12 mois
void initLapins(long long couples) {
    int a;
    for (a = 0; a <= AGE_MAX; a++) {
        fem[a] = 0;
        mal[a] = 0;
    }
    fem[12] = couples;
    mal[12] = couples;
    naissances = 0;
    morts = 0;
}

// simule un mois : naissances, puis vieillissement et mort
void simMois() {
    int a;
    long long femMat = 0, malMat = 0, portees, chatons = 0, femelles, i;

    // naissances (calculees avant le vieillissement)
    for (a = MAT_MIN; a < AGE_MAX; a++) {
        femMat += (long long)(fem[a] * partMat(a));
        malMat += (long long)(mal[a] * partMat(a));
    }
    if (malMat > 0 && femMat > 0) {
        portees = binomiale(femMat, 0.5);          // 6 portees par an en moyenne
        if (portees <= 50) {
            for (i = 0; i < portees; i++) chatons += alea(BEBES_MIN, BEBES_MAX);
        } else {                                   // somme de nombreuses portees
            chatons = (long long)(4.5 * portees + sqrt(1.25 * portees) * normale() + 0.5);
            if (chatons < 0) chatons = 0;
        }
    }
    naissances = chatons;
    femelles = binomiale(chatons, 0.5);            // 50 % de femelles
    fem[0] = femelles;
    mal[0] = chatons - femelles;

    // vieillissement et mort (on part de la fin du tableau)
    morts = 0;
    for (a = AGE_MAX; a >= 1; a--) {
        fem[a] = binomiale(fem[a - 1], survieCohorte(a));
        mal[a] = binomiale(mal[a - 1], survieCohorte(a));
        morts += (fem[a - 1] - fem[a]) + (mal[a - 1] - mal[a]);
    }
    fem[0] = 0;
    mal[0] = 0;
}

// affiche une ligne du tableau d'evolution
void afficheLigne(int mois, long long nesAn, long long mortsAn) {
    printf("%5.1f %12lld %12lld %12lld %13lld %13lld %7.1f\n",
           mois / 12.0, total(), totalFem(), totalMal(), nesAn, mortsAn, ageMoyen());
}

// repartition des vivants par tranche d'age
void afficheAges() {
    long long t = total();
    long long nb;
    int i, a;
    int debut[6] = {0, 6, 12, 60, 120, 181};
    printf("Vivants par tranche d'age :\n");
    for (i = 0; i < 5; i++) {
        nb = 0;
        for (a = debut[i]; a < debut[i + 1]; a++) nb += fem[a] + mal[a];
        printf("   %3d a %3d mois : %12lld (%5.1f %%)\n",
               debut[i], debut[i + 1] - 1, nb, t > 0 ? 100.0 * nb / t : 0.0);
    }
}

// lance une experience
// retourne le mois ou 1 milliard est atteint, -1 = extinction, -2 = cible non atteinte
int simLapins(long long couples, int verbeux) {
    int mois;
    long long tot, nesAn = 0, mortsAn = 0;

    initLapins(couples);
    moisDix = 0;

    if (verbeux) {
        printf("%5s %12s %12s %12s %13s %13s %7s\n",
               "Annee", "Vivants", "Femelles", "Males", "Naissances", "Morts", "AgeMoy");
    }

    for (mois = 1; mois <= MOIS_MAX; mois++) {
        simMois();
        nesAn += naissances;
        mortsAn += morts;
        tot = total();
        if (moisDix == 0 && tot >= DIX_MILLIONS) moisDix = mois;

        // une ligne par annee, et une ligne a l'arret
        if (verbeux && (mois % 12 == 0 || tot == 0 || tot >= CIBLE)) {
            afficheLigne(mois, nesAn, mortsAn);
            nesAn = 0;
            mortsAn = 0;
        }
        if (tot == 0) return -1;
        if (tot >= CIBLE) return mois;
    }
    return -2;
}

int main() {
    int i, e, res, nbExt, nbRate, nbOk, nbDix, mini, maxi;
    int departs[3] = {1, 5, 10};
    int resultats[NB_EXP];
    double somme, sommeDix, moyenne, variance, ecart;
    clock_t debut;

    srand((unsigned)time(NULL));

    // test 1 : une simulation detaillee avec 1 couple
    printf("=== Test 1 : 1 couple, cohortes, evolution annuelle ===\n");
    debut = clock();
    res = simLapins(1, 1);
    printf("\n");
    if (res > 0) printf("1 milliard atteint au mois %d (%d ans et %d mois)\n", res, res / 12, res % 12);
    else if (res == -1) printf("Extinction de la population\n");
    else printf("Cible non atteinte en %d mois\n", MOIS_MAX);
    if (moisDix > 0) printf("10 millions atteints au mois %d\n", moisDix);
    printf("Temps : %.3f s\n", (double)(clock() - debut) / CLOCKS_PER_SEC);
    afficheAges();

    // test 2 : 30 experiences pour chaque condition initiale
    printf("\n=== Test 2 : %d experiences par condition ===\n", NB_EXP);
    for (i = 0; i < 3; i++) {
        nbExt = 0; nbRate = 0; nbOk = 0; nbDix = 0;
        somme = 0; sommeDix = 0; mini = 100000; maxi = 0;
        debut = clock();

        for (e = 0; e < NB_EXP; e++) {
            res = simLapins(departs[i], 0);
            if (res == -1) nbExt++;
            else if (res == -2) nbRate++;
            else {
                resultats[nbOk] = res;
                nbOk++;
                somme += res;
                if (res < mini) mini = res;
                if (res > maxi) maxi = res;
            }
            if (moisDix > 0) {
                nbDix++;
                sommeDix += moisDix;
            }
        }

        printf("\n%d couple(s) au depart :\n", departs[i]);
        printf("   Extinctions : %d / %d\n", nbExt, NB_EXP);
        printf("   1 milliard atteint : %d / %d\n", nbOk, NB_EXP);
        if (nbOk > 1) {
            moyenne = somme / nbOk;
            variance = 0;
            for (e = 0; e < nbOk; e++) {
                variance += (resultats[e] - moyenne) * (resultats[e] - moyenne);
            }
            ecart = sqrt(variance / (nbOk - 1));
            printf("   Mois moyen pour 1 milliard : %.1f (ecart-type %.2f, min %d, max %d)\n",
                   moyenne, ecart, mini, maxi);
        }
        if (nbDix > 0) printf("   Mois moyen pour 10 millions : %.1f\n", sommeDix / nbDix);
        printf("   Temps : %.3f s\n", (double)(clock() - debut) / CLOCKS_PER_SEC);
    }
    return 0;
}