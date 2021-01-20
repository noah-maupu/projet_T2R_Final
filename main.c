#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TicketToRideAPI.h"

/*
li1417-56.members.linode.com
port 1234
port 5678
*/
typedef struct{
	int nbChemin;		//nombre de chemin a prendre pour completer la carte objectif
	int ville[20];		//ville par lesquelle il faut passer pour completer la carte objectif
	int done;			// indique si les deux villes sont ralliées  0 : non rallier, 1 : rallier,  2 : 
	int presque;		//indique le nombre de chemin restant a prendre
}t_chemin_plus_court;

typedef struct{
	int nbWagons;				//nombre de wagons
	int nbCards;				//nombre de cartes
	int cards[10];				//nombre de cartes de chaque couleur, ie cards[YELLOW] iest le nombre de cartes jaunes
	int nbObjectives;			//nombre d'objectifs
	t_chemin_plus_court	chemin[20]; 			
	t_objective objectives[20];	//objectifs, valide uniquement avec l'index <nbObjectives>  	
}t_player;

typedef struct{
	char name[20]; 		//nom du jeu
	t_color faceUp[5]; 	//cartes faces visibles
	int tour;			//indique qui doit jouer    0 pour moi, 1 pour l'adversaire	
	t_player players[2];
}t_game;

typedef struct{
	int length; 			//longueur de la track
	t_color color1, color2;	//colors
	int taken; 	//indique si le chemin est pris par moi (1), par l'adversaire (2), pas prise (0)
}t_tracks;



typedef struct{
	int nbCities;
	int nbTracks;
	int* p_tab_tracks;
}t_plateau;

/*typedef struct{
	int ville1;
	int ville2;
	int color;
	int nbLocomotives;
	int num_carte_draw;
	int num_carte_objective;
}t_choix;*/

void affichage_structure(t_plateau plateau, t_game game)
{
	printf("\n\n");
	printf("affichage de la structure plateau\n");
	printf("nombre de villes: %d, nombre de tracks: %d\n", plateau.nbCities, plateau.nbTracks);
	/*for (int i = 0; i < 5*plateau.nbTracks; ++i)
	{
		printf("%d  ", plateau.p_tab_tracks[i]);
	}*/

	printf("affichage de la structure de mon joueur\n");
	printf("nbWagons: %d\n", game.players[0].nbWagons);
	printf("nbCards: %d\n", game.players[0].nbCards);
	printf("nbObjectives: %d\n", game.players[0].nbObjectives);
	printf("nombre de cartes de chaque couleur\n");
	for (int i = 0; i < 10; ++i)
	{
		printf("%d\n", game.players[0].cards[i]);
	}
	for (int i = 0; i < 20; ++i)
	{
		printf("%d %d %d\n", game.players[0].objectives[i].city1, game.players[0].objectives[i].city2, game.players[0].objectives[i].score);
	}
	printf("affichage de la structure de l'adversaire\n");
	printf("nbWagons: %d\n", game.players[1].nbWagons);
	printf("nbCards: %d\n", game.players[1].nbCards);
	printf("nbObjectives: %d\n", game.players[1].nbObjectives);
	printf("nombre de cartes de chaque couleur\n");
	for (int i = 0; i < 10; ++i)
	{
		printf("%d\n", game.players[1].cards[i]);
	}

	printf("affichage les tracks\n");
	for (int i = 0; i < plateau.nbTracks; ++i)
	{
		for (int j = 0; j < 5; j++)
		{
			printf("%d ", plateau.p_tab_tracks[(i*5)+j]);
		}
		printf("\n");
		
	}
	printf("\n\n");


}

//initialise la partie
void init_partie(t_game* game, t_plateau* plateau, t_color first_cards[4])
{
	game->players[0].nbObjectives=0;
	game->players[0].nbCards=4;
	game->players[1].nbCards=4;
	game->tour=0;
	for (int i = 0; i < 10; ++i)
	{
		game->players[0].cards[i]=0;
		game->players[1].cards[i]=0;
	}

	for (int i = 0; i < 20; ++i)
	{
		game->players[0].objectives[i].city1=0;
		game->players[0].objectives[i].city2=0;
		game->players[0].objectives[i].score=0;
	}

	waitForT2RGame("TRAINING NICE_BOT", game->name, &plateau->nbCities, &plateau->nbTracks);
	
}

//fonction qui indique qui a gagné
void fin_de_partie(t_return_code move, t_game game)
{
	if ((move==WINNING_MOVE && game.tour==0) || (move==LOOSING_MOVE && game.tour==1))
	{
		printf("J'ai gagné\n");
	}
	else if((move==WINNING_MOVE && game.tour==1) || (move==LOOSING_MOVE && game.tour==0))
	{
		printf("j'ai perdu\n");
	}
}

void askMove(t_move* move){
	/* ask for the type */
	printf("Choose a move\n");
	printf("1. claim a route\n");
	printf("2. draw a blind card\n");
	printf("3. draw a card\n");
	printf("4. draw objectives\n");
	printf("5. choose objectives\n");
	int move_type;
	scanf("%d", &move_type);
	move->type=move_type;
	int choix_color;

	/* ask for details */
	if (move->type == CLAIM_ROUTE) {
		printf("Give city1, city2, color and nb of locomotives: ");
		scanf("%d %d %d %d", &move->claimRoute.city1, &move->claimRoute.city2, &choix_color,
			  &move->claimRoute.nbLocomotives);
		move->claimRoute.color=choix_color;
		//printf("\nlengh : %d", tab_2D_routes[move->claimRoute.city1][move->claimRoute.city2].length);
	}
	else if (move->type == DRAW_CARD) {
		printf("Give the color: ");
		scanf("%d", &choix_color);
		move->drawCard.card=choix_color;
	}
	else if (move->type == CHOOSE_OBJECTIVES){
		printf("For each objective, give 0 or 1:");
		scanf("%d %d %d", &move->chooseObjectives.chosen[0], &move->chooseObjectives.chosen[1], &move->chooseObjectives.chosen[2]);
	}

}

void bot_choice(t_move* move, t_plateau* plateau, t_game* game, t_tracks tab_2D_routes[100][100], int replay)
{
	int compt_multi =0; //compteur qui me permet de piocher une carte multicolore lorsque ca fait 5 tours que je dois piocher et qu'il y a une carte multicolore

	int indice[5]; //indique l'indice de la carte objectif à compléter
	for (int i = 0; i < 5; ++i)
	{
		indice[i] = 0;
	}
	int compt = 0;
	for (int i = 1; i < game->players[0].nbObjectives+1; ++i)
	{
		//printf("chemin fait : %d\n", game->players[0].chemin[i].done);
		if (game->players[0].chemin[i].done == 0)
		{
			indice[compt] =i;
			compt++; 
		}
	}

	//printf("nombre de carte non fini : %d\n", compt);

	// si jamais je dois rejouer, soit je pioche soit je prend une carte
	if (replay == 1) //replay est égale à 1 uniquement si on doit repiocher
	{

		for (int i = 0; i < compt; ++i) // fais autant de tour qu'il y a de carte objectif à compléter
		{
			for (int j = 0; j < game->players[0].chemin[indice[i]].nbChemin; ++j) //fais autant de tour qu'il y a de chemin à prendre entre les deux villes de l'objectif
			{
				//si le chemin à prendre est pas pris, on regarde si on peut prendre une carte de la même couleur que le chemin sinon on pioche
				if (tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].taken == 0) 
				{
					for (int i = 0; i < 5; ++i) //on prend la carte de la meme couleur que le chemin si elle existe
					{
						if (game->faceUp[i] != 9 && game->faceUp[i] == tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].color1)
						{
							move->type = DRAW_CARD;
							move->drawCard.card = tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].color1;
							return;
						}
					}
					
					move->type = DRAW_BLIND_CARD; // sinon on pioche
					return;
				}
			}
		}
	}
	//permet de piocher les cartes objectifs uniquement si j'ai fini toutes mes cartes objectifs et si il reste assez de wagons à chacun
	else if (compt==0 && game->players[1].nbWagons >10 && game->players[0].nbWagons > 15) // si tous les chemins sont fait on prend une carte objectif
	{
		for (int i = 0; i < game->players[0].nbObjectives+1; ++i)
		{
			if (compt == 0 || (compt == 1 && game->players[0].chemin[i].presque == 1))
			{
				move->type = DRAW_OBJECTIVES;
				return;
			}
		}
		
	}
	else 
	{	//permet de regarder si on peut prendre une route qui nous avance dans nos cartes objectifs seulement si les deux joueurs ont plus de 15 wagons
		if ( game->players[1].nbWagons > 15 && game->players[0].nbWagons > 15) 
		{
			for (int i = 0; i < compt; ++i) // autant de boucle que de carte objectifs non fini
			{
				for (int j = 0; j < game->players[0].chemin[indice[i]].nbChemin; ++j) // autant de boucle que de chemin à prendre pour la carte
				{
					//printf("chemin que l'on regarde : %d -> %d\n", game->players[0].chemin[indice[i]].ville[j], game->players[0].chemin[indice[i]].ville[j+1]);
					//printf("couleur du chemin : %d\n", tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].color1);
					//printf("la route est prise : %d\n", tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].taken);
					if (tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].taken == 0) 
						//on essaye de prendre un chemin si on peut 
					{
						//si le chemin n'as pas de couleur
						if (tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].color1 == 9)
						{
							int most_card = 1;
							// on regarde la couleur de la carte avec le plus de carte
							for (int k = 1; k < 9; ++k)
							{	
								if (game->players[0].cards[k] >= game->players[0].cards[most_card])
								{
									most_card = k;
								}
							}
							//printf("couleur de la carte : %d\n", most_card);
							//printf("nombre de card %d : %d\n", most_card, game->players[0].cards[most_card]);
							
							//on prend le chemin avec cette couleur de carte si on peut
							//si mon nombre de carte de cette couleur + le nombre de carte multicolore est supérieur à la taille du chemin
							if ((game->players[0].cards[most_card] + game->players[0].cards[MULTICOLOR]) >= tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].length)
							{
								//printf("on essaye de prendre la route multi\n");
								move->type=CLAIM_ROUTE;
								move->claimRoute.city1 = game->players[0].chemin[indice[i]].ville[j];
								move->claimRoute.city2 = game->players[0].chemin[indice[i]].ville[j+1];
								move->claimRoute.color = most_card;
								//printf("couleur pour prendre le chemin%d\n", move->claimRoute.color);
								int nbLoco =  tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]]
								[game->players[0].chemin[indice[i]].ville[j+1]].length
								- game->players[0].cards[most_card];
								if (nbLoco<0) //nbLoco représente le nombre de carte multicolore que l'on va utiliser
								{
									nbLoco=0;
								}
								move->claimRoute.nbLocomotives = nbLoco;
								return;
							}
						}
						//si le chemin a une couleur on essaye de la prendre
						else if (game->players[0].cards[tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].color1]
							+ game->players[0].cards[MULTICOLOR] >= 
							tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].length)
						{
							//printf("on essaye de prendre la route\n");
							move->type=CLAIM_ROUTE;
							move->claimRoute.city1 = game->players[0].chemin[indice[i]].ville[j];
							move->claimRoute.city2 = game->players[0].chemin[indice[i]].ville[j+1];
							move->claimRoute.color = tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].color1;
							int nbLoco =  tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]]
							[game->players[0].chemin[indice[i]].ville[j+1]].length
							 - game->players[0].cards[tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].color1];
								if (nbLoco<0)
								{
									nbLoco=0;
								}
							move->claimRoute.nbLocomotives = nbLoco;
							return;
						}
					}
				}
			}
		}
		
		else //on essaye de prendre une route aléatoire si les joueurs ont moins de 15 wagons
		{
			//printf("on regarde si on peut prendre une route aléatoire\n");
			//printf("plus de wagons\n");

			//recherche d'un chemin que l'on peut prendre
			for (int i = 0; i < 100; ++i)
			{
				for (int j = 0; j < 100; ++j)
				{
					//si la route existe, on essaye de prendre la route
					if (tab_2D_routes[i][j].taken==0 && tab_2D_routes[i][j].length > 0)
					{
						//printf("chemin que l'on regarde : %d -> %d\n", i, j);
						//printf("longueur : %d\n", tab_2D_routes[i][j].length);
						//printf("couleur du chemin : %d\n", tab_2D_routes[i][j].color1);
						//printf("la route est prise : %d\n", tab_2D_routes[i][j].taken);

						//si le chemin n'as pas de couleur
						if (tab_2D_routes[i][j].color1 == 9)
						{
							int most_card = 1;
							// on regarde la couleur de la carte avec le plus de carte
							for (int k = 1; k < 9; ++k)
							{	
								if (game->players[0].cards[k] >= game->players[0].cards[most_card])
								{
									most_card = k;
								}
							}
							////printf("couleur de la carte : %d\n", most_card);
							////printf("nombre de carte de most : %d\n", game->players[0].cards[most_card]);
							////printf("nombre de locomotive : %d\n", game->players[0].cards[MULTICOLOR]);
							
							//on essaye de prendre le chemin avec cette couleur de carte
							if (game->players[0].cards[most_card] + game->players[0].cards[MULTICOLOR] >= tab_2D_routes[i][j].length)
							{
								//printf("on essaye de prendre la route multi\n");
								move->type=CLAIM_ROUTE;
								move->claimRoute.city1 = i;
								move->claimRoute.city2 = j;
								move->claimRoute.color = most_card;
								//printf("couleur pour prendre le chemin%d\n", move->claimRoute.color);
								int nbLoco =  tab_2D_routes[i][j].length
								- game->players[0].cards[most_card];
								if (nbLoco<0)
								{
									nbLoco=0;
								}
								move->claimRoute.nbLocomotives = nbLoco;
								return;
							}
						}
						//si la route a eu une couleur, on essaye de prendre la route
						else if (game->players[0].cards[tab_2D_routes[i][j].color1]
							+ game->players[0].cards[MULTICOLOR] >= 
							tab_2D_routes[i][j].length)
						{
							//printf("on essaye de prendre la route\n");
							move->type=CLAIM_ROUTE;
							move->claimRoute.city1 = i;
							move->claimRoute.city2 = j;
							move->claimRoute.color = tab_2D_routes[i][j].color1;
							int nbLoco =  tab_2D_routes[i][j].length
							 - game->players[0].cards[tab_2D_routes[i][j].color1];
								if (nbLoco<0)
								{
									nbLoco=0;
								}
							move->claimRoute.nbLocomotives = nbLoco;
							return;
						}
					}
				}
			}	
		}
		//comme on a pris aucun chemin, on pioche en rapport avec le premier chemin à prendre
		
		//printf("on prend une carte\n");
		for (int i = 0; i < compt; ++i)
		{
			for (int j = 0; j < game->players[0].chemin[indice[i]].nbChemin; ++j)
			{
				//printf("chemin que l'on regarde : %d -> %d\n", game->players[0].chemin[indice[i]].ville[j], game->players[0].chemin[indice[i]].ville[j+1]);
				//printf("couleur du chemin : %d\n", tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].color1);
				
				if (tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].taken == 0) 
				{
					//le chemin n'est pas pris donc on essaye de prendre une carte de la meme couleur que le chemin
					for (int k = 0; k < 5; ++k) //on prend la carte de la meme couleur que le chemin si elle existe
					{
						if (game->faceUp[k] == tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].color1)
						{
							move->type = DRAW_CARD;

							move->drawCard.card = tab_2D_routes[game->players[0].chemin[indice[i]].ville[j]][game->players[0].chemin[indice[i]].ville[j+1]].color1;
							return;
						}
					}
					for (int l = 0; l < 5; ++l) // sinon on prend une carte MULTICOLOR si elle existe toute les 5 fois où on a pu la prendre
					{
						if (game->faceUp[l] == MULTICOLOR)
						{
							compt_multi +=1;
							if (compt_multi == 5)
							{
								move->type = DRAW_CARD;
								move->drawCard.card = MULTICOLOR;
								compt_multi=0;
							return;
							}
							
						}
					}
				}
			}
		}
	move->type = DRAW_BLIND_CARD; // sinon on pioche
	return;
	}
}

/* plays the move given as a parameter (send to the server)
 * returns the return code */
t_return_code playOurMove(t_move* move, t_color* lastCard){
	t_return_code ret;

	switch (move->type) {
		case CLAIM_ROUTE:
			ret = claimRoute(move->claimRoute.city1, move->claimRoute.city2, move->claimRoute.color, move->claimRoute.nbLocomotives);
			*lastCard = NONE;
			break;
		case DRAW_CARD:
			ret = drawCard(move->drawCard.card, move->drawCard.faceUp);
			*lastCard = (*lastCard==NONE && move->drawCard.card!= MULTICOLOR) ? move->drawCard.card : NONE;
			break;
		case DRAW_BLIND_CARD:
			ret = drawBlindCard(&move->drawBlindCard.card);
			*lastCard = (*lastCard==NONE) ? move->drawBlindCard.card : NONE;
			break;
		case DRAW_OBJECTIVES:
			ret = drawObjectives(move->drawObjectives.objectives);
			for(int i=0; i<3; i++){
				//printf("%d. %d (", i, move->drawObjectives.objectives[i].city1);
				printCity(move->drawObjectives.objectives[i].city1);
				//printf(") -> (");
				printCity(move->drawObjectives.objectives[i].city2);
				//printf(") %d (%d pts)\n", move->drawObjectives.objectives[i].city2, move->drawObjectives.objectives[i].score);
			}
			*lastCard = NONE;
			break;
		case CHOOSE_OBJECTIVES:
			ret = chooseObjectives(move->chooseObjectives.chosen);
			*lastCard = NONE;
			break;
	}

	return ret;
}

/* tell if we need to replay */
int needReplay(t_move* move, t_color lastCard){
	int replay = 0;

	//if (move->type == DRAW_OBJECTIVES)
	//	replay = 1;
	if (move->type == DRAW_BLIND_CARD && lastCard == NONE)
		replay = 1;
	else if (move->type == DRAW_CARD && lastCard == NONE && move->drawCard.card != MULTICOLOR)
		replay = 1;

	return replay;
}

void play(t_move* move)
{
	if (move->type==1)
	{
		claimRoute(move->claimRoute.city1, move->claimRoute.city2, move->claimRoute.color, move->claimRoute.nbLocomotives);
	}
	else if (move->type==2)
	{
		drawBlindCard(&move->drawBlindCard.card);
	}

	else if (move->type==3)
	{
		drawCard(move->drawCard.card, move->drawCard.faceUp);
	}

	else if (move->type==4)
	{
		drawObjectives(move->drawObjectives.objectives);
	}
	else if (move->type==5)
	{
		chooseObjectives(move->chooseObjectives.chosen);
	}
}

// Cette fonction initialise le tableau à double entrée qui contient les données de toutes les routes 
// tab_2D_routes est le tableau qui contient les données de toutes les routes, en indice, il faut rentrer les deux villes qui sont reliées par une route
void init_tab_route(t_plateau* plateau, t_tracks tab_2D_routes[100][100])
{
	for (int i = 0; i < 100; ++i)
	{
		for (int j = 0; j < 100; ++j)
		{
			tab_2D_routes[i][j].length = 0;
			//tab_2D_routes[i][j].taken = 3;
		}
	}
	for (int i = 0; i < plateau->nbTracks; ++i)
	{
		tab_2D_routes[plateau->p_tab_tracks[(i*5)]][plateau->p_tab_tracks[(i*5)+1]].length=plateau->p_tab_tracks[(i*5)+2];
		tab_2D_routes[plateau->p_tab_tracks[(i*5+1)]][plateau->p_tab_tracks[(i*5)]].length=plateau->p_tab_tracks[(i*5)+2];

		tab_2D_routes[plateau->p_tab_tracks[(i*5)]][plateau->p_tab_tracks[(i*5)+1]].color1=plateau->p_tab_tracks[(i*5)+3];
		tab_2D_routes[plateau->p_tab_tracks[(i*5+1)]][plateau->p_tab_tracks[(i*5)]].color1=plateau->p_tab_tracks[(i*5)+3];

		tab_2D_routes[plateau->p_tab_tracks[(i*5)]][plateau->p_tab_tracks[(i*5)+1]].color2=plateau->p_tab_tracks[(i*5)+4];
		tab_2D_routes[plateau->p_tab_tracks[(i*5+1)]][plateau->p_tab_tracks[(i*5)]].color2=plateau->p_tab_tracks[(i*5)+4];

	}
}


//Cette fonction range les 4 premières cartes que l'on récupère en début de partie dans la structure game.players
void range_carte_main(t_color cards[4], t_game* game)
{
	for (int i = 0; i < 4 ; ++i)
	{
		game->players[0].cards[cards[i]]++;
	}
}

int distanceMini(int D[50], int visite[50])
{
	int min = 1000;
	int indice_min;
	for (int i = 0; i < 50; ++i)
	{
		if (visite[i] == 0 && D[i] < min)
		{
			min = D[i];
			indice_min = i;
		}
	}
	return indice_min;
}

//fonction qui classe les longueurs du plus petit au plus grand et les indices correspondant des trois cartes objectifs que l'on pioche
//longueur_tot est le tableau qui contient les longueurs et les indices
void rangement_longueur(int longueur_tot[6])
{
	int l0 = longueur_tot[0];
	int l1 = longueur_tot[1];
	int l2 = longueur_tot[2];
	int ind0 = longueur_tot[3];
	int ind1 = longueur_tot[4];
	int ind2 = longueur_tot[5];

	if (l0 < l1)
	{
		if (l0 < l2)
		{
			longueur_tot[0] = l0;
			longueur_tot[3] = ind0;
			if (l1 < l2)
			{
				longueur_tot[1] = l1;
				longueur_tot[4] = ind1;
				longueur_tot[2] = l2;
				longueur_tot[5] = ind2;
			}
			else
			{
				longueur_tot[1] = l2;
				longueur_tot[4] = ind2;
				longueur_tot[2] = l1;
				longueur_tot[5] = ind1;
			}
		}
		else
		{
			longueur_tot[0] = l2;
			longueur_tot[3] = ind2;
			longueur_tot[1] = l0;
			longueur_tot[4] = ind0;
			longueur_tot[2] = l1;
			longueur_tot[5] = ind1;
		}
	}
	else if (l1 < l0)
	{
		if (l1 < l2)
		{
			longueur_tot[0] = l1;
			longueur_tot[3] = ind1;
			if (l0 < l2)
			{
				longueur_tot[1] = l0;
				longueur_tot[4] = ind0;
				longueur_tot[2] = l2;
				longueur_tot[5] = ind2;
			}
			else
			{
				longueur_tot[1] = l2;
				longueur_tot[4] = ind2;
				longueur_tot[2] = l0;
				longueur_tot[5] = ind0;
			}
		}
		else
		{
			longueur_tot[0] = l2;
			longueur_tot[3] = ind2;
			longueur_tot[1] = l1;
			longueur_tot[4] = ind1;
			longueur_tot[2] = l0;
			longueur_tot[5] = ind0;
		}
	}
}

//fonction qui trouve le chemin le plus court entre deux villes et qui prend en compte les routes que j'ai prise et celles prises par l'adversaire
void le_plus_court_chemin(int src, int dest, t_tracks tab_2D_routes[100][100], int D[50], int Prec[50])
{
	/*int source, destination;
	if (src > dest)
	{
		source = dest;
		destination = src;
	}
	else
	{
		source = src;
		destination = dest;
	}*/
	int visite[50];
	for (int i = 0; i < 50; ++i)
	{
		D[i]=10000;
		visite[i]=0;
	}
	D[src]=0;
	int u=src;
	int length;
	while(u!=dest)
	{
		u=distanceMini(D, visite);
		visite[u]=1;
		for (int v = 0; v < 50; ++v)
		{
			if (visite[v]==0 && tab_2D_routes[u][v].length != 0 
				&& tab_2D_routes[u][v].taken != 2)
	 		{
	 			if (tab_2D_routes[u][v].taken == 0)
	 			{
	 				length = tab_2D_routes[u][v].length;
	 			}
	 			else if (tab_2D_routes[u][v].taken == 1)
	 			{
	 				length = 0;
	 			}
	 			if (D[u]+length < D[v])
	 			{
	 				D[v] = D[u] + length;
					Prec[v] = u;
					
	 			}
			}
		}
	}
}

//Fonction qui calcule le nombre de wagons nécessaires pour prendre les routes qui relies les deux villes d'une cartes objectif
int calcule_long(int src, int dest, int Prec[50], t_tracks tab_2D_routes[100][100])
{
	int longueur_tot=0;
	int v = dest;
	while (v!=src)
	{
		longueur_tot += tab_2D_routes[v][Prec[v]].length;
		v=Prec[v];
	}
	return longueur_tot;
}

//Fonction qui affiche les villes par lesquelle on doit passer pour prendre une carte objectif
void affiche_le_chemin(int src, int dest, int Prec[50])
{
	int v = dest;
	printf("%d -> ", v);
	while(v != src)
	{
		printf("%d -> ", Prec[v]);
		v = Prec[v];
	}
	
}

//fonction qui met à jour les structures à chaque coup jouer
void mise_a_jour_move(t_move* move, t_tracks tab_2D_routes[100][100], t_game* game, t_plateau* plateau, t_objective obj[3], int joueur, int ind_chosen)
{
	//printf("game->tour : %d\n", game->tour);
	//si le coup jouer est claimRoute
	if (move->type == 1)
	{
		//on met à jour le tableau tab_2D_routes pour connaitre qui à quelle route
		if (game->tour ==0)
		{
			tab_2D_routes[move->claimRoute.city1][move->claimRoute.city2].taken = 1;
			tab_2D_routes[move->claimRoute.city2][move->claimRoute.city1].taken = 1;
		}
		else if (game->tour == 1)
		{
			tab_2D_routes[move->claimRoute.city1][move->claimRoute.city2].taken = 2;
			tab_2D_routes[move->claimRoute.city2][move->claimRoute.city1].taken = 2;
		}
		//on déduit toutes les cartes utilisés pour prendre la route
		game->players[game->tour].nbCards -= tab_2D_routes[move->claimRoute.city1][move->claimRoute.city2].length;
		game->players[game->tour].nbWagons -= tab_2D_routes[move->claimRoute.city1][move->claimRoute.city2].length;
		game->players[game->tour].cards[MULTICOLOR] -= move->claimRoute.nbLocomotives;
		game->players[game->tour].cards[move->claimRoute.color] -= (tab_2D_routes[move->claimRoute.city1][move->claimRoute.city2].length - move->claimRoute.nbLocomotives); 
		
		//on met à jour la structure players[0] pour savoir si une cartes objectifs est compléter
		if (game->tour == 0)
		{
			int presque=0; // si presque >=1 alors la carte objectif n'est pas compléter
			for (int i = 1; i < game->players[0].nbObjectives+1; ++i)
			{
				for (int j = 0; j < game->players[0].chemin[i].nbChemin; ++j)
				{
					//printf("chemin à mettre à jour%d, %d\n",game->players[0].chemin[i].ville[j], game->players[0].chemin[i].ville[j+1]);
					//printf("taken : %d\n", tab_2D_routes[game->players[0].chemin[i].ville[j]][game->players[0].chemin[i].ville[j+1]].taken);
					if (tab_2D_routes[game->players[0].chemin[i].ville[j]][game->players[0].chemin[i].ville[j+1]].taken != 1)
					{
						presque += 1;
					}
					if (tab_2D_routes[game->players[0].chemin[i].ville[j]][game->players[0].chemin[i].ville[j+1]].taken == 2)
					{
						//printf("§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§\n");
					}
					//printf("presque : %d\n", presque);
					if (presque == 0)
					{
						game->players[0].chemin[i].done = 1;
					}		
					else
					{
						game->players[0].chemin[i].done = 0;
						game->players[0].chemin[i].presque = presque;
					}
					
				}
				presque = 0;
		
			}
		}
		//si c'est l'adversaire qui a pris une route, on vérifie si la route qui la prise fesait parti et notre itinéraire et si oui on recalcule le plus court chemin et on l'enregistre
		else if (game->tour == 1)
		{
			for (int i = 1; i < game->players[0].nbObjectives+1; ++i)
			{
				for (int j = 0; j < game->players[0].chemin[i].nbChemin; ++j)
				{
					//printf("ville 1 : %d\n", game->players[0].chemin[i].ville[j]);
					//printf("ville 2 : %d\n", game->players[0].chemin[i].ville[j+1]);
					if (move->claimRoute.city1 == game->players[0].chemin[i].ville[j] && 
						move->claimRoute.city2 == game->players[0].chemin[i].ville[j+1])
					{
						int D[50];
						int Prec[50];
						le_plus_court_chemin(game->players[0].objectives[i].city1, game->players[0].objectives[i].city2, tab_2D_routes, D, Prec);
						//printf("mise à jour chemin le plus court\n");

						// j'enregistre le chemin
						int v = game->players[0].objectives[i].city2;
						game->players[0].chemin[i].ville[0]=v;
						int k = 1;
						while (v!=game->players[0].objectives[i].city1)
						{
							game->players[0].chemin[i].ville[k]=Prec[v];
							v=Prec[v];
							k++;
						}
						game->players[0].chemin[i].nbChemin=k-1;
						//affiche_le_chemin(game->players[0].objectives[i].city1, game->players[0].objectives[i].city2, Prec);
					}
				}
			}
		}
			
	}
	//si une carte à été piocher, on met à jour la structure qui contient le nombre de cartes de chaque couleur
	else if (move->type == 2)
	{
		if(game->tour == 0)
		{
			game->players[game->tour].cards[move->drawBlindCard.card]++;
		}
		game->players[game->tour].nbCards++;
	}

	//si une carte à été prise
	else if (move->type == 3)
	{
		game->players[game->tour].cards[move->drawCard.card]++;
		game->players[game->tour].nbCards++;	
		for (int i = 0; i < 5; ++i)
		{
			game->faceUp[i] = move->drawCard.faceUp[i];
		}
		
	}

	//on enregistre la carte objective qui a été conservé
	else if (move->type == 5)
	{
		if(game->tour==0)
		{
			if (move->chooseObjectives.chosen[ind_chosen]==1)
			{
				game->players[game->tour].objectives[game->players[game->tour].nbObjectives].city1 = obj[ind_chosen].city1;
				game->players[game->tour].objectives[game->players[game->tour].nbObjectives].city2 = obj[ind_chosen].city2;
				game->players[game->tour].objectives[game->players[game->tour].nbObjectives].score = obj[ind_chosen].score;
				//game->players[game->tour].nbObjectives ++;
			
			}
		}
	}
}


//Fonction qui enregistre le chemin le plus court dans la structure
void enregistrement_chemin_le_plus_court(int src, int dest, int Prec[50], t_game* game)
{
	/*int destination = dest;
	int source = src;
	if (src < dest)
	{
		destination = src;
		source = dest;
	}*/
	int v = dest;
	game->players[0].chemin[game->players[0].nbObjectives].ville[0]=v;
	int i = 1;
	while(v != src)
	{
		game->players[0].chemin[game->players[0].nbObjectives].ville[i]=Prec[v];
		//printf("Prec[v] :%d\n", Prec[v]);
		v=Prec[v];
		i++;
		if (i > 20)
	{
		//printf("erreur\n");
		return;
	}
	}
	
	game->players[0].chemin[game->players[0].nbObjectives].nbChemin = (i-1);
	//printf("nbChemin : %d\n", i-1);
}

int main(int argc, char const *argv[])
{
	connectToServer("li1417-56.members.linode.com", 1234, "hano_1_bot");

	//do{

		t_plateau plateau;
		t_game game;
		t_color first_cards[4]; // tableau qui contient les premières cartes que l'on a 
		init_partie(&game, &plateau, first_cards);
		int* arrayTracks = malloc(5*plateau.nbTracks*sizeof(int));
		game.tour=getMap(arrayTracks, game.faceUp, first_cards);
		int premier_tour=0;	//indique si c'est le premier tour
		game.players[0].nbWagons=45;
		game.players[1].nbWagons=45;
		plateau.p_tab_tracks=arrayTracks;
		t_tracks tab_2D_routes[100][100];

		int D[50];
		int Prec[50];
		init_tab_route(&plateau, tab_2D_routes);

		t_return_code code_move; //variables qui nous dit si le mouvement jouer à permit à quelqu'un de gagner
		t_move opp_move; //variable qui contient les données du coup de l'adversaire
		t_move my_move; //variable qui contient les données de mon coup
		t_color lastCard;
		t_objective obj[3]; // tableau qui contient les données des cartes objectifs piochées
		int replay;


		for (int i = 0; i < 20; ++i)
		{
			game.players[0].chemin[i].done = 0;
		}

		range_carte_main(first_cards, &game);

		do{
			if (!replay)
			{
				printMap();
				//printf("nombre de  locomotives : %d\n\n\n", game.players[0].cards[MULTICOLOR]);
			}

			//tour de l'adversaire
			if (game.tour==1)
			{
				code_move=getMove(&opp_move, &replay);
				//printf("move_opp : %d\n", opp_move.type);
				mise_a_jour_move(&opp_move, tab_2D_routes, &game, &plateau, obj, game.tour, 0);
				
			}

			//mon tour

			else if (game.tour == 0)
			{
				//si c'est mon premier tour, on prend au moins de cartes objectif
				if (premier_tour == 0) // on fait le premier tour ou on doit choisir deux cartes objectifs
				{
					my_move.type = 4;
					//printf("%d\n", my_move.type);
					code_move = playOurMove(&my_move, &lastCard);
					// choix pré-définie pour prendre les deux premieres cartes objectifs
					for (int i = 0; i < 3; ++i)
					{
						obj[i].city1=my_move.drawObjectives.objectives[i].city1;
						obj[i].city2=my_move.drawObjectives.objectives[i].city2;
						obj[i].score=my_move.drawObjectives.objectives[i].score;
					}
					my_move.chooseObjectives.chosen[0] = 1;
					my_move.chooseObjectives.chosen[1] = 1;
					my_move.chooseObjectives.chosen[2] = 0;

					my_move.type = 5;
					//Pour les deux cartes prises, on calcule le chemin le plus court et on l'enregistre
					game.players[0].nbObjectives = 1;
					le_plus_court_chemin(obj[0].city1, obj[0].city2, 
						tab_2D_routes, D, Prec);
					//printf("objectif n° : 1 \n");
					enregistrement_chemin_le_plus_court(obj[0].city1, 
						obj[0].city2, Prec, &game);
					//printf("nbChemin(main) : %d\n", game.players[0].chemin[1].nbChemin);
					/*for (int i = 0; i < game.players[0].chemin[1].nbChemin; ++i)
					{
						printf("ville :%d -> ", game.players[0].chemin[1].ville[i]);
					}*/
					//printf("derniere ville :%d\n", game.players[0].chemin[1].ville[game.players[0].chemin[1].nbChemin]);
					mise_a_jour_move(&my_move, tab_2D_routes, &game, &plateau, obj, game.tour,0);
					
					game.players[0].nbObjectives += 1;
					le_plus_court_chemin(obj[1].city1, obj[1].city2, 
						tab_2D_routes, D, Prec);
					//printf("objectif n° : 2 \n");
					enregistrement_chemin_le_plus_court(obj[1].city1, 
						obj[1].city2, Prec, &game);
					//printf("nbChemin(main) : %d\n", game.players[0].chemin[2].nbChemin);
					/*for (int i = 0; i < game.players[0].chemin[2].nbChemin; ++i)
					{
						printf("ville :%d -> ", game.players[0].chemin[2].ville[i]);
					}*/
					//printf("derniere ville :%d\n", game.players[0].chemin[2].ville[game.players[0].chemin[2].nbChemin]);
					mise_a_jour_move(&my_move, tab_2D_routes, &game, &plateau, obj, game.tour,1);
					code_move = playOurMove(&my_move, &lastCard);

					premier_tour = 1;
				}
				//sinon le bot choisit un coup
				else
				{
					//askMove(&my_move);
					bot_choice(&my_move, &plateau, &game,tab_2D_routes, replay);
					//printf("choix du move : %d\n", my_move.type);
					replay = needReplay(&my_move, lastCard);
					code_move = playOurMove(&my_move, &lastCard);
					//Si le choix est de piocher des cartes objectifs, on voit quelle cartes on garde
					if (my_move.type==4)
					{
						int longueur_tot[6];
						for (int i = 0; i < 3; ++i)
						{
							obj[i].city1=my_move.drawObjectives.objectives[i].city1;
							obj[i].city2=my_move.drawObjectives.objectives[i].city2;
							obj[i].score=my_move.drawObjectives.objectives[i].score;
							//Pour chaque objectif, on calcule le chemin le plus court et on les classes
							le_plus_court_chemin(obj[i].city1, obj[i].city2, 
								tab_2D_routes, D, Prec);
							longueur_tot [i] = calcule_long(obj[i].city1, obj[i].city2, Prec, tab_2D_routes);
							longueur_tot[i+3]=i;
							//affiche_le_chemin(obj[i].city1, obj[i].city2,  Prec);
						}
						/*for (int i = 0; i < 6; ++i)
							{
								//printf("%d\n", longueur_tot[i]);
							}*/
						rangement_longueur(longueur_tot);
						if (longueur_tot[0] > game.players[0].nbWagons) // je prend la carte avec la plus petite longueur si par malheur, le nombre de wagons est inférieur au nombre de wagonq requis pour miniser la perte de points
						{
							my_move.chooseObjectives.chosen[longueur_tot[3]] = 1;
							my_move.chooseObjectives.chosen[longueur_tot[4]] = 0;
							my_move.chooseObjectives.chosen[longueur_tot[5]] = 0;
							game.players[0].nbObjectives += 1;
							my_move.type = 5;
							le_plus_court_chemin(obj[longueur_tot[3]].city1, obj[longueur_tot[3]].city2, 
								tab_2D_routes, D, Prec);
							enregistrement_chemin_le_plus_court(obj[longueur_tot[3]].city1, obj[longueur_tot[3]].city2, Prec, &game);
							mise_a_jour_move(&my_move, tab_2D_routes, &game, &plateau, obj, game.tour, longueur_tot[3]);
						}
						else // sinon je prend la carte avec le plus long chemin si l'adversaire à plus de 15 wagons et que le nombre de wagons requis +5 est inférieur à mon nombre de wagons
						{
							if (longueur_tot[3] + 5 < game.players[0].nbWagons && game.players[1].nbCards >15)
							{
								my_move.chooseObjectives.chosen[longueur_tot[3]] = 0;
								my_move.chooseObjectives.chosen[longueur_tot[4]] = 0;
								my_move.chooseObjectives.chosen[longueur_tot[5]] = 1;
								my_move.type = 5;
								game.players[0].nbObjectives += 1;
								le_plus_court_chemin(obj[longueur_tot[3]].city1, obj[longueur_tot[3]].city2, 
								tab_2D_routes, D, Prec);
								enregistrement_chemin_le_plus_court(obj[longueur_tot[3]].city1, obj[longueur_tot[3]].city2, Prec, &game);
								mise_a_jour_move(&my_move, tab_2D_routes, &game, &plateau, obj, game.tour,longueur_tot[3]);
								for (int i = 0; i < game.players[0].chemin[game.players[0].nbObjectives].nbChemin; ++i)
								{
									//printf("ville :%d -> ", game.players[0].chemin[game.players[0].nbObjectives].ville[i]);
								}
								//printf("derniere ville :%d\n", game.players[0].chemin[game.players[0].nbObjectives].ville[game.players[0].chemin[game.players[0].nbObjectives].nbChemin]);
								
							}
							//sinon on prend la carte avec le chemin le plus court
							else 
							{

								my_move.chooseObjectives.chosen[longueur_tot[3]] = 1;
								my_move.chooseObjectives.chosen[longueur_tot[4]] = 0;
								my_move.chooseObjectives.chosen[longueur_tot[5]] = 0;
								game.players[0].nbObjectives += 1;
								my_move.type = 5;
								le_plus_court_chemin(obj[longueur_tot[3]].city1, obj[longueur_tot[3]].city2, 
								tab_2D_routes, D, Prec);
								enregistrement_chemin_le_plus_court(obj[longueur_tot[3]].city1, obj[longueur_tot[3]].city2, Prec, &game);
								mise_a_jour_move(&my_move, tab_2D_routes, &game, &plateau, obj, game.tour, longueur_tot[3]);						
							}
						}
						code_move = playOurMove(&my_move, &lastCard);
					}
				

				}
				//on met à jour les structures si c'est le bot qui a jouer ou que le coup est différents de chooseObjectives car dans le cas contraire, elles sont déjà mise à jour 
				if (my_move.type != 5 && game.tour !=1)
				{
					mise_a_jour_move(&my_move, tab_2D_routes, &game, &plateau, obj, game.tour, 0);
				}
				//affichage_structure(plateau, game);

				

			}
			//si le joueur n'as plus besoin de rejouer, c'est au tour de l'adversaire
			if (code_move == NORMAL_MOVE && !replay)
			{
				game.tour=!game.tour;
			}

			
		}while(code_move==NORMAL_MOVE);
		
		//on indique qui à gagner
		fin_de_partie(code_move, game);

	//}while(1);
	closeConnection();
	return 0;
}