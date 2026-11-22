#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define QNT_FORMIGAS 3
#define DELIMITADOR_FIM_SARRAFO -1
#define CORTE_NAO_CABE NULL
#define SUCESSO 0

#define PESO_HEURISTICA 0.5f
#define PESO_FEROMONIO 0.5f
#define FEROMONIO_POR_FORMIGA 1.0f
#define TAXA_EVAPORACAO 0.1f

typedef struct {
	int quantidade;
	int tamanho; //(cm)
} Corte;

/*
No das listas usadas pela estrutura de Formigas
prox: ponteiro para o próximo elemento da lista
ant: ponteiro para o elemento anterior da lista
dado: armazena o valor naquele no
*/
typedef struct NoLista {
	struct NoLista* prox;
	struct NoLista* ant;
	int dado;
} NoLista;

/*
Estrutura de cada uma das formigas
ultima_formiga: ponteiro para o último resultado da formiga
ultima_sobra: ponteiro para o ultimo resultado de sobra da formiga
qnt_sarrafos_formiga: quantos sarrafos a formiga usou
*/
typedef struct {
	NoLista* ultima_formiga;
	NoLista* ultima_sobra;
	int qnt_sarrafos_formiga;
} Formiga;

void push_sobras(Formiga* formigas, int sobra, int idx) {
	NoLista* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) exit(EXIT_FAILURE);

	if (!formigas[idx].ultima_sobra) {
		formigas[idx].ultima_sobra = novo_no;
		novo_no->ant = NULL;
	}
	else {
		formigas[idx].ultima_sobra->prox = novo_no;
		novo_no->ant = formigas[idx].ultima_sobra;
		formigas[idx].ultima_sobra = novo_no;
	}

	novo_no->prox = NULL;
	novo_no->dado = sobra;
}

void push_formiga(Formiga* formigas, int dado, int* tamanho_restante_sarrafo, int tamanho_sarrafo, int idx) {
	NoLista* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) exit(EXIT_FAILURE);

	if (!formigas[idx].ultima_formiga) {
		formigas[idx].ultima_formiga = novo_no;
		novo_no->ant = NULL;
		push_sobras(formigas, tamanho_sarrafo, idx);
	}
	else {
		formigas[idx].ultima_formiga->prox = novo_no;
		novo_no->ant = formigas[idx].ultima_formiga;
		formigas[idx].ultima_formiga = novo_no;
	}

	novo_no->prox = NULL;

	novo_no->dado = (dado > *tamanho_restante_sarrafo) ? DELIMITADOR_FIM_SARRAFO : dado;

	if (novo_no->dado == DELIMITADOR_FIM_SARRAFO) {
		*tamanho_restante_sarrafo = tamanho_sarrafo;
		push_formiga(formigas, dado, tamanho_restante_sarrafo, tamanho_sarrafo, idx);
		push_sobras(formigas, *tamanho_restante_sarrafo, idx);
		++formigas[idx].qnt_sarrafos_formiga;
		return;
	}

	*tamanho_restante_sarrafo -= dado;

	(formigas[idx].ultima_sobra)->dado = *tamanho_restante_sarrafo;

	if (!*tamanho_restante_sarrafo) {
		*tamanho_restante_sarrafo = tamanho_sarrafo;
		++formigas[idx].qnt_sarrafos_formiga;

		NoLista* fim_sarrafo = malloc(sizeof(*fim_sarrafo));
		if (!fim_sarrafo) exit(EXIT_FAILURE);
		fim_sarrafo->dado = DELIMITADOR_FIM_SARRAFO;
		fim_sarrafo->prox = NULL;
		fim_sarrafo->ant = formigas[idx].ultima_formiga;
		formigas[idx].ultima_formiga->prox = fim_sarrafo;
		formigas[idx].ultima_formiga = fim_sarrafo;


		push_sobras(formigas, tamanho_sarrafo, idx);

	}
}


void init_formigas(Formiga* formigas, int qnt_formigas) {
	memset(formigas, 0, qnt_formigas * sizeof(*formigas));
	for (int i = 0; i < qnt_formigas; ++i) formigas[i].qnt_sarrafos_formiga = 1;
}

void init_matriz_cortes(int* matriz_cortes, int qnt_cortes, Corte* cortes) {
	for (int i = 0; i < qnt_cortes; ++i) {
		for (int j = 0; j < qnt_cortes; ++j) {
			matriz_cortes[j + (i * qnt_cortes)] = cortes[j].tamanho;
		}
	}
}

void init_matriz_feromonios(float* feromonios, int qnt_cortes) {
	for (int i = 0; i < qnt_cortes * qnt_cortes; ++i) {
		feromonios[i] = 1.0f;
	}
}


void atualizar_feromonios(float* __restrict feromonios, float taxa_evaporacao, int qnt_cortes) {
	for (int i = 0; i < qnt_cortes * qnt_cortes; ++i) feromonios[i] -= feromonios[i] * taxa_evaporacao;
}

void probabilidade_transicao(float* chance_visitar, float* feromonios, const float peso_feromonio, const float* matriz_cortes, const float peso_heuristica, int qnt_cortes) {
	float soma = 0.0f;
	for (int i = 0; i < qnt_cortes; ++i) {
		soma += powf(matriz_cortes[i], peso_heuristica) * pow(feromonios[i], peso_feromonio);
	}

	for (int i = 0; i < qnt_cortes; ++i) {
		chance_visitar[i] = (pow(feromonios[i], peso_feromonio) * pow(matriz_cortes[i], peso_heuristica)) / soma;
	}
}


int escolha_transicao(float* chance_visitar, int qnt_cortes) {
	float num_sorteio = (float)(rand() % 10000) / 10000.0f;

	float chances_acumuladas = 0.0f;
	int i;
	for (i = 0; i < qnt_cortes; ++i) {
		chances_acumuladas += chance_visitar[i];
		if (chances_acumuladas && chances_acumuladas >= num_sorteio) break;
	}

	return i;
}

void print_sarrafos(Formiga* formigas, int qnt_formigas, int idx) {
	for (int i = 0; i < qnt_formigas; ++i) {
		printf("Formiga %d\n", i + 1);

		NoLista* no_formiga_atual = formigas[i].ultima_formiga;
		NoLista* no_sobra_atual = formigas[i].ultima_sobra;

		if (!no_formiga_atual || !no_sobra_atual) continue;

		while (no_formiga_atual->ant) no_formiga_atual = no_formiga_atual->ant;
		while (no_sobra_atual->ant) no_sobra_atual = no_sobra_atual->ant;

		for (int num_sarrafo = 1; num_sarrafo <= formigas[i].qnt_sarrafos_formiga; ++num_sarrafo) {

			if (!no_formiga_atual) break;

			if (num_sarrafo != 1) printf("\n");
			printf("Cortes sarrafo %d: ", num_sarrafo);
			while (no_formiga_atual) {
				if (no_formiga_atual->dado == DELIMITADOR_FIM_SARRAFO) {
					no_formiga_atual = no_formiga_atual->prox;
					break;
				}
				printf("%d ", no_formiga_atual->dado);
				no_formiga_atual = no_formiga_atual->prox;
			}


			if (no_sobra_atual) {
				printf("\nSobra sarrafo %d: %d\n", num_sarrafo, no_sobra_atual->dado);
				no_sobra_atual = no_sobra_atual->prox;
			}
		}
		printf("\n");
	}
}


void construirtourf(Formiga* formigas, Corte* cortes, float* feromonios, const float peso_feromonio, const float* matriz_cortes, const float peso_heuristica, int qnt_cortes, int tamanhosarrafo) {
	float chance_visitar[3] = { 0.0f, 0.0f, 0.0f };

	int total_cortes = 0;
	for (int k = 0; k < qnt_cortes; ++k) {
		total_cortes += cortes[k].quantidade;
	}

	for (int i = 0; i < QNT_FORMIGAS; i++) {
		int tamanho_restante = tamanhosarrafo;

		Corte cortes_restantes[3];
		memcpy(cortes_restantes, cortes, qnt_cortes * sizeof(Corte));

		for (int j = 0; j < qnt_cortes; j++) {
			probabilidade_transicao(chance_visitar, feromonios, peso_feromonio, matriz_cortes, peso_heuristica, qnt_cortes);

			float soma_valida = 0.0f;
			for (int k = 0; k < qnt_cortes; ++k) {
				if (cortes_restantes[k].quantidade <= 0) {
					chance_visitar[k] = 0.0f;
				}
				soma_valida += chance_visitar[k];
			}
			for (int k = 0; k < qnt_cortes; ++k) {
				if (soma_valida > 0.0f) chance_visitar[k] /= soma_valida;
			}

			int esc = escolha_transicao(chance_visitar, qnt_cortes);

			int proxcorte = cortes[esc].tamanho;
			cortes_restantes[esc].quantidade--;

			push_formiga(formigas, proxcorte, &tamanho_restante, tamanhosarrafo, i);

		}

		if (tamanho_restante == tamanhosarrafo && formigas[i].qnt_sarrafos_formiga > 1) {
			--formigas[i].qnt_sarrafos_formiga;
			NoLista* sobra_fantasma = formigas[i].ultima_sobra;
			if (sobra_fantasma && sobra_fantasma->ant) {
				formigas[i].ultima_sobra = sobra_fantasma->ant;
				formigas[i].ultima_sobra->prox = NULL;
				free(sobra_fantasma);
			}
		}


	}

}


int encontrar_melhor_formiga(Formiga* formigas, int qnt_formigas) {
	int melhor_idx = 0;

	for (int i = 1; i < qnt_formigas; ++i) {

		if (formigas[i].qnt_sarrafos_formiga < formigas[melhor_idx].qnt_sarrafos_formiga) {
			melhor_idx = i;
		}

		else if (formigas[i].qnt_sarrafos_formiga == formigas[melhor_idx].qnt_sarrafos_formiga) {
			if (formigas[i].ultima_sobra && formigas[melhor_idx].ultima_sobra) {
				if (formigas[i].ultima_sobra->dado > formigas[melhor_idx].ultima_sobra->dado) {
					melhor_idx = i;
				}
			}
		}
	}


	printf("\n>>> A melhor formiga foi a Formiga %d (indice %d) <<<\n", melhor_idx + 1, melhor_idx);
	printf("Sarrafos usados: %d\n", formigas[melhor_idx].qnt_sarrafos_formiga);
	if (formigas[melhor_idx].ultima_sobra) {
		printf("Sobra do ultimo sarrafo: %d cm\n", formigas[melhor_idx].ultima_sobra->dado);
	}

	return melhor_idx;
}



int main() {

	int qnt_cortes = 3;
	int tamanho_S = 60;
	Corte cortes[3] = { { 3, 10 },{ 4, 20 },{ 5, 30 } };
	Formiga formigas[QNT_FORMIGAS];
	init_formigas(formigas, QNT_FORMIGAS);
	const float peso_heuristica = 0.5f;
	const float peso_feromonio = 0.5f;
	const float feromonio_por_formiga = 1.0f;
	const float taxa_evaporacao = 0.1f;

	int* matriz_cortes = malloc(qnt_cortes * qnt_cortes * sizeof(*matriz_cortes));
	if (!matriz_cortes) {
		puts("Problema na alocacao de memoria para a matriz de cortes");
		return 1;
	}

	init_matriz_cortes(matriz_cortes, qnt_cortes, cortes);

	float* feromonios = malloc(qnt_cortes * qnt_cortes * sizeof(*feromonios));
	if (!feromonios) {
		puts("Problema na alocacao de memoria para matriz de feromonios");
		free(matriz_cortes);
		return 1;
	}

	init_matriz_feromonios(feromonios, qnt_cortes);
	float chance_visitar[3] = { 0.0f, 0.0f, 0.0f };

	probabilidade_transicao(chance_visitar, feromonios, peso_feromonio, matriz_cortes, peso_heuristica, qnt_cortes);

	int esc = escolha_transicao(chance_visitar, qnt_cortes);


	construirtourf(formigas, cortes, feromonios, peso_feromonio, matriz_cortes, peso_heuristica, qnt_cortes, tamanho_S);

	print_sarrafos(formigas, QNT_FORMIGAS, 0);

	int melhor_idx = encontrar_melhor_formiga(formigas, QNT_FORMIGAS);


	return 0;
}



/*
int esc = escolha_transicao(chance_visitar, qnt_cortes);




*/


/*
float soma_teste = 0.0f;
for (int i = 0; i < qnt_cortes; ++i) {
	printf("Corte %d (tam %d): chance = %.4f (%.2f%%)\n", i, cortes[i].tamanho, chance_visitar[i], chance_visitar[i] * 100);
	soma_teste += chance_visitar[i];
}
printf("Soma total: %.4f\n", soma_teste);



*/

/*
Corte 0 (tam 10): chance = 0.2412 (24.12%)
Corte 1 (tam 20): chance = 0.3411 (34.11%)
Corte 2 (tam 30): chance = 0.4177 (41.77%)
Soma total: 1.0000


*/