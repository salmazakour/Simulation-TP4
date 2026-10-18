#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

#define MAX 10000000       // population maximale (environ 30 Mo)
#define MAT_MIN 5         // maturite minimale (mois)
#define MAT_MAX 8         // maturite maximale (mois)
#define BEBES_MIN 3       // bebes minimum par portee
#define BEBES_MAX 6       // bebes maximum par portee
#define NB_EXP 30         // nombre d'experiences par etude
#define DUREE 84          // duree des etudes en mois (7 ans)

// un lapin
struct Lapin {
    uint16_t age;
    uint8_t sexe;         // 0 = femelle, 1 = male
    uint8_t mat;          // age de maturite
    uint8_t portees;      // nombre de portees par an
    uint8_t deja;         // portees deja faites cette annee
};

struct Lapin pop[MAX];    // population de lapins

int n = 0;                // nombre de lapins vivants
int plein = 0;            // 1 si on ne peut plus ajouter de lapin
int nbMorts = 0;          // nombre de morts
long sommeAgeMort = 0;    // somme des ages a la mort
long mortsParAge[181];    // nombre de morts a chaque age (en mois)
double survieJeune = 0.35; // survie annuelle des jeunes (parametre a tester)
int resultat = 0;         // 0 = extinction, 1 = MAX atteint, 2 = duree complete
int moisArret = 0;        // mois ou l'experience s'est arretee

// entier aleatoire entre a et b
int alea(int a, int b) {
    return a + rand() % (b - a + 1);
}

// reel aleatoire entre 0 et 1
double proba() {
    return rand() / (RAND_MAX + 1.0);
}

// nombre de portees par an (3 a 9), plus de chances pour 5, 6, 7
int tirePortees() {
    int r = alea(1, 18);
    if (r <= 1) return 3;
    if (r <= 3) return 4;
    if (r <= 7) return 5;
    if (r <= 11) return 6;
    if (r <= 15) return 7;
    if (r <= 17) return 8;
    return 9;
}

// ajoute un lapin, si le tableau est plein on le signale
void ajoute(int sexe, int age) {
    if (n < MAX) {
        pop[n].age = age;
        pop[n].sexe = sexe;
        pop[n].mat = alea(MAT_MIN, MAT_MAX);
        if (sexe == 0) {
            pop[n].portees = tirePortees();
        } else {
            pop[n].portees = 0;
        }
        pop[n].deja = 0;
        n++;
    } else {
        plein = 1;
    }
}

// le lapin i survit-il ce mois-ci ? (1 = oui, 0 = non)
int survit(int i) {
    double ans = pop[i].age / 12.0;
    double annuel;
    if (pop[i].age >= 180) return 0;                       // 15 ans : mort
    if (pop[i].age < pop[i].mat) annuel = survieJeune;     // jeune
    else if (ans < 10) annuel = 0.60;                      // adulte
    else annuel = 0.60 * (15 - ans) / 5;                   // vieux : baisse jusqu'a 0
    return proba() < pow(annuel, 1.0 / 12);                // taux annuel -> mensuel
}

// simule un mois
void pasMois() {
    int nAvant = n;
    int males = 0;
    int i, j, k, cycle, restants, a_faire;
    double p;

    // compter les males matures
    for (i = 0; i < nAvant; i++) {
        if (pop[i].sexe == 1 && pop[i].age >= pop[i].mat) males++;
    }

    // naissances
    if (males > 0) {
        for (i = 0; i < nAvant; i++) {
            if (plein) break;   // on ne peut plus ajouter : on arrete
            if (pop[i].sexe == 0 && pop[i].age >= pop[i].mat) {
                cycle = (pop[i].age - pop[i].mat) % 12;   // mois dans l'annee (0 a 11)
                if (cycle == 0) pop[i].deja = 0;           // nouvelle annee
                restants = 12 - cycle;                     // mois restants
                a_faire = pop[i].portees - pop[i].deja;    // portees qui restent
                if (a_faire > 0) {
                    p = (double)a_faire / restants;
                    if (p > 1) p = 1;
                    if (proba() < p) {
                        pop[i].deja++;
                        k = alea(BEBES_MIN, BEBES_MAX);    // nombre de bebes
                        for (j = 0; j < k; j++) {
                            if (proba() < 0.5) ajoute(0, 0);   // femelle
                            else ajoute(1, 0);                 // male
                        }
                    }
                }
            }
        }
    }

    // vieillissement et mort (un mort est remplace par le dernier lapin)
    i = 0;
    while (i < n) {
        pop[i].age++;
        if (survit(i)) {
            i++;
        } else {
            nbMorts++;
            sommeAgeMort += pop[i].age;
            mortsParAge[pop[i].age]++;
            pop[i] = pop[n - 1];
            n--;
        }
    }
}

// simulation complete, s'arrete si extinction ou si MAX est atteint
// renvoie la population finale, resultat et moisArret donnent la raison de l'arret
// si afficher = 1, affiche les vivants et les morts de chaque annee
int simLapins(int couples, int mois, int afficher) {
    int c, t, a;
    int mortsAvant = 0;    // nombre de morts au debut de l'annee en cours

    n = 0;
    plein = 0;
    nbMorts = 0;
    sommeAgeMort = 0;
    for (a = 0; a <= 180; a++) mortsParAge[a] = 0;
    for (c = 0; c < couples; c++) {
        ajoute(0, 12);   // femelle adulte
        ajoute(1, 12);   // male adulte
    }

    if (afficher) {
        printf("%6s %12s %16s %14s\n", "Annee", "Vivants", "Morts (annee)", "Morts (total)");
    }

    for (t = 1; t <= mois; t++) {
        pasMois();

        // affichage a la fin de chaque annee
        if (afficher && t % 12 == 0) {
            printf("%6d %12d %16d %14d\n", t / 12, n, nbMorts - mortsAvant, nbMorts);
            mortsAvant = nbMorts;
        }

        if (plein) {               // plus de place : on arrete
            resultat = 1;
            moisArret = t;
            if (afficher && t % 12 != 0) {
                printf("%4d m %12d %16d %14d  (arret)\n", t, n, nbMorts - mortsAvant, nbMorts);
            }
            return n;
        }
        if (n == 0) {              // extinction
            resultat = 0;
            moisArret = t;
            if (afficher && t % 12 != 0) {
                printf("%4d m %12d %16d %14d  (arret)\n", t, n, nbMorts - mortsAvant, nbMorts);
            }
            return 0;
        }
    }
    resultat = 2;
    moisArret = mois;
    return n;
}

// repartition des vivants par tranche d'age
void afficheAges() {
    int i, a;
    long nb;
    int debut[6] = {0, 6, 12, 60, 120, 181};
    printf("Vivants par tranche d'age :\n");
    for (i = 0; i < 5; i++) {
        nb = 0;
        for (a = 0; a < n; a++) {
            if (pop[a].age >= debut[i] && pop[a].age < debut[i + 1]) nb++;
        }
        printf("   %3d a %3d mois : %ld\n", debut[i], debut[i + 1] - 1, nb);
    }
}

// repartition des morts par tranche d'age
void afficheMorts() {
    int i, a;
    long nb;
    int debut[6] = {0, 6, 12, 60, 120, 181};
    printf("Morts par tranche d'age :\n");
    for (i = 0; i < 5; i++) {
        nb = 0;
        for (a = debut[i]; a < debut[i + 1]; a++) nb += mortsParAge[a];
        printf("   %3d a %3d mois : %ld\n", debut[i], debut[i + 1] - 1, nb);
    }
}

// etude statistique : NB_EXP experiences avec les memes parametres
void etude(int couples, int mois) {
    int e, final, extinctions = 0, limites = 0, vivants = 0;
    int min = MAX, max = 0;
    int finals[NB_EXP];
    double somme = 0, sommeMoisLimite = 0, moyenne, variance = 0, ecart, marge, bas;
    clock_t debut = clock();

    for (e = 0; e < NB_EXP; e++) {
        final = simLapins(couples, mois, 0);
        if (resultat == 0) {
            extinctions++;
        } else if (resultat == 1) {
            limites++;
            sommeMoisLimite += moisArret;
        } else {
            finals[vivants] = final;
            vivants++;
            somme += final;
            if (final < min) min = final;
            if (final > max) max = final;
        }
    }

    printf("\n=== %d couple(s), survie jeunes %.2f, %d experiences, %d mois max ===\n",
           couples, survieJeune, NB_EXP, mois);
    printf("Extinctions : %d / %d (%.1f %%)\n", extinctions, NB_EXP,
           100.0 * extinctions / NB_EXP);
    printf("MAX (%d lapins) atteint : %d / %d", MAX, limites, NB_EXP);
    if (limites > 0) printf(" (mois moyen d'arret : %.1f)", sommeMoisLimite / limites);
    printf("\n");
    printf("Duree complete : %d / %d\n", vivants, NB_EXP);

    if (vivants >= 2) {
        moyenne = somme / vivants;
        for (e = 0; e < vivants; e++) {
            variance += (finals[e] - moyenne) * (finals[e] - moyenne);
        }
        ecart = sqrt(variance / (vivants - 1));
        marge = 1.96 * ecart / sqrt(vivants);    // intervalle de confiance a 95 %
        bas = moyenne - marge;
        if (bas < 0) bas = 0;
        printf("Population finale moyenne : %.0f (ecart-type %.0f)\n", moyenne, ecart);
        printf("Minimum : %d, Maximum : %d\n", min, max);
        printf("Intervalle de confiance a 95 %% : [%.0f ; %.0f]\n", bas, moyenne + marge);
    } else {
        printf("Pas assez d'experiences a duree complete pour un intervalle\n");
    }
    printf("Temps : %.2f s\n", (double)(clock() - debut) / CLOCKS_PER_SEC);
}

int main() {
    int i;
    int couples[3] = {1, 5, 10};
    double survies[3] = {0.25, 0.35, 0.45};
    clock_t debut;

    srand(time(NULL));

    // test 1 : une simulation detaillee, 5 couples, 20 ans max
    printf("=== Test 1 : 1 couple, 20 ans max ===\n");
    debut = clock();
    simLapins(1, 240, 1);
    if (resultat == 0) printf("Extinction au mois %d\n", moisArret);
    else if (resultat == 1) printf("MAX atteint au mois %d\n", moisArret);
    else printf("Duree complete (%d mois)\n", moisArret);
    printf("Temps : %.2f s\n", (double)(clock() - debut) / CLOCKS_PER_SEC);
    if (nbMorts > 0) {
        printf("Morts : %d, age moyen a la mort : %.1f mois\n",
               nbMorts, (double)sommeAgeMort / nbMorts);
    }
    afficheAges();
    afficheMorts();

    // test 2 : impact du nombre de couples au depart
    survieJeune = 0.35;
    for (i = 0; i < 3; i++) {
        etude(couples[i], DUREE);
    }

    // test 3 : impact de la survie des jeunes (5 couples au depart)
    for (i = 0; i < 3; i++) {
        survieJeune = survies[i];
        etude(5, DUREE);
    }
    survieJeune = 0.35;

    return 0;
}