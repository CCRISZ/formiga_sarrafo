#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DELIMITADOR_FIM_SARRAFO -1
#define CORTE_NAO_CABE NULL
#define SUCESSO 0

typedef struct {
	int* quantidade;
	int* tamanho;
} Cortes;

typedef struct NoLista{
	struct NoLista* prox;
	int dado;
} NoLista;

typedef struct {
	NoLista* inicio_formiga;
	NoLista* ultima_formiga;
	NoLista* inicio_sobra;
	NoLista* ultima_sobra;
	int* qnt_sarrafos_formiga;
} Formigas;

void push_sobras(Formigas* formigas, int sobra) {
	NoLista* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) exit(EXIT_FAILURE);

	if (!formigas->ultima_sobra) formigas->ultima_sobra = formigas->inicio_sobra = novo_no;
	else {
		formigas->ultima_sobra->prox = novo_no;
		formigas->ultima_sobra = novo_no;
	}

	novo_no->prox = NULL;
	novo_no->dado = sobra;
}

void push_formiga(Formigas* formigas, int dado, int* tamanho_restante_sarrafo, int tamanho_sarrafo, int idx) {
	NoLista* novo_no = malloc(sizeof(*novo_no));
	if (!novo_no) exit(EXIT_FAILURE);

	if (!formigas->ultima_formiga) formigas->ultima_formiga = formigas->inicio_formiga = novo_no;
	else {
		formigas->ultima_formiga->prox = novo_no;
		formigas->ultima_formiga = novo_no;
	}

	novo_no->prox = NULL;

	novo_no->dado = (dado > *tamanho_restante_sarrafo) ? DELIMITADOR_FIM_SARRAFO : dado;

	if (novo_no->dado == DELIMITADOR_FIM_SARRAFO) {
		push_sobras(formigas, *tamanho_restante_sarrafo);
		return;
	}

	*tamanho_restante_sarrafo -= dado;

	if (!*tamanho_restante_sarrafo) {
		push_sobras(formigas, 0);
		*tamanho_restante_sarrafo = tamanho_sarrafo;
		++formigas->qnt_sarrafos_formiga[idx];
	}
}

int main(void) {
	//teste das funções
	Formigas teste = { .qnt_sarrafos_formiga = malloc(3 * sizeof(int))};
	for (int i = 0; i < 3; ++i) teste.qnt_sarrafos_formiga[i] = 1;
	int tamanho = 3;
	push_formiga(&teste, 1, &tamanho, 3, 0);
	push_formiga(&teste, 2, &tamanho, 3, 0);
	push_formiga(&teste, 4, &tamanho, 3, 0);


	int tam_sarrafos;
	printf("Insira o tamanho dos sarrafos (cm): ");
	scanf("%d", &tam_sarrafos);

	Cortes cortes;

	int qnt_cortes;
	printf("Insira a quantidade de cortes: ");
	scanf("%d", &qnt_cortes);

	cortes.quantidade = malloc(qnt_cortes * sizeof(*cortes.quantidade));
	cortes.tamanho = malloc(qnt_cortes * sizeof(*cortes.tamanho));
	if (!cortes.quantidade || !cortes.tamanho) {
		puts("problema na alocacao de memoria para os cortes");

		free(cortes.tamanho);
		free(cortes.quantidade);

		return 1;
	}

	puts("Insira o tamanho do corte (cm) e a quantidade separado por um espaco");
	for (int i = 0; i < qnt_cortes; ++i) {
		scanf("%d %d", cortes.tamanho + i, cortes.quantidade + i);
	}

	free(cortes.tamanho);
	free(cortes.quantidade);

	return 0;
}