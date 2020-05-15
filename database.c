#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DATA 512
#define MAX_ROWS 100

struct Endereco{
	int id;
	int set;
	char nome[MAX_DATA];
	char logradouro[MAX_DATA];
	char cep[MAX_DATA];
	char bairro[MAX_DATA];
	char cidade[MAX_DATA];
};

struct Database{
	//cria a variável rows que recebe dados do struct Endereço
	struct Endereco rows[MAX_ROWS];
};

struct Connection{
	/*
	  cria dois ponteiros que apontam para structs, um da 
	  API interna do C, que é o FILE, e o outro para o
  	  Database	
	*/
	FILE *file;
	struct Database *db;
};

// cria a função que recebe a mensagem de erro, imprime e mata o código

void kill(const char *message)
{
	if(errno){
		perror(message);
	}else{
		printf("ERROR: %s\n",message);
	}
	exit(1);
}

// cria a função que imprime o Endereço

void Imprime_endereco(struct Endereco *addr)
{
	printf("%d %s %s %s %s %s",addr->id,addr->nome,addr->logradouro,addr->bairro,addr->cep,addr->cidade);

}

// cria a função que carrega o banco de dados

void Database_load(struct Connection *conn)
{
	int read_count = fread(conn->db,sizeof(struct Database),1,conn->file);
		if(read_count != 1)
			kill("Falha no carregamento do Banco de dados");

}

// cria a função que abre a conexão com o banco de dados e faz a alocação de memória para a Conexão e para a Base de Dados em Si, como foi usado malloc ele vai armazenar a memória em HEAP, embora a tendencia fosse em STACK

struct Connection  *Database_open(const char *filename, char mode)
{
	struct Connection *conn = malloc(sizeof(struct Connection));
		if(!conn)
			kill("Erro de Memória");
		conn->db = malloc(sizeof(struct Database));
		if(!conn->db)
			kill("Erro de memória");

		if(mode == 'c'){
			conn->file = fopen(filename,"w");
		}else{
			conn->file = fopen(filename,"r+");
			if(conn->file)
				Database_load(conn);
		}

		if(!conn->file)
			kill("Erro ao abrir o arquivo.");
		return conn;
}

// cria a função que fecha a conexão com o banco de dados e libera a memória alocada no HEAP do SO

void Database_close(struct Connection *conn)
{
	if(conn){
		if(conn->file)
			fclose(conn->file);
		if(conn->db)
			free(conn->db);
		free(conn);
	}
}

// cria a função de escrita no banco de dados utilizando o rewind()

void Database_write(struct Connection *conn)
{
	rewind(conn->file);
	int read_count = fwrite(conn->db,sizeof(Database),1,conn->file);
	
	if(read_count != 1)
		kill("Falha ao gravar na base de dados.");
	
	read_count = fflush(conn->file);

	if(read_count == -1)
		kill("Não pode descarregar base de dados");	
}

// função que cria o banco de dados de fato

void Database_create(struct Connection *conn)
{
	int i = 0;
	for(i = 0; i < MAX_ROWS; i++){
		
		//cria um prototype para inicializa-lo
		struct Endereco addr = {.id = id, .set = 0};
		
		//atribui o prototype
		conn->db->rows[i] = addr;

	};
}

// 

void Database_set(struct Connection *conn, int id, const char *nome, const char *logradouro, const char *cep, const char *bairro, const char *cidade)
{
		struct Endereco *addr = &conn->db->rows[id];

		if(addr->set)
			kill("Já configurado, delete ele primeiro.");

		addr->set = 1;

		//ATENÇÂO
		
		char *res = strncpy(addr->nome,nome,MAX_DATA);

		// Constata o bug do strncpy
		
		if(!res)
			kill("Falha ao copiar o nome.");

		res = strncpy(addr->logradouro,logradouro,MAX_DATA);

		if(!res)
			kill("Falha ao copiar o logradouro.");

		res = strncpy(addr->cep,cep,MAX_DATA);

		if(!res)
			kill("Falha ao copiar o cep.");

		res = strncpy(addr->bairro,bairro,MAX_DATA);

		if(!res)
			kill("Falha ao copiar o bairro.");

		res = strncpy(addr->data,data,MAX_DATA);

		if(!res)
			kill("Falha ao copiar a data.");
		
}	


// função que faz o get do CRUD

void Database_get(struct Connection *conn, int id)
{
	struct Endereco *addr = &conn->db->rows[id];

	if(addr->set){
		Imprime_endereco(addr);
	}else{
		kill("ID não está configurada.");
	}
}

// função que faz o delete do CRUD

void Database_delete(struct Connection *conn, int id)
{
	struct Endereco addr = {.id = id , .set = 0};
	conn->db->rows[id] = addr;
}

void Database_list(struct Connection *conn)
{
	int i = 0 ;
	struct Database *db = conn->db;

	for(i = 0; i < MAX_ROWS; i++){
		struct Endereco *cur = &db->rows[i];
		if(cur->set){
			Imprime_endereco(cur);
		}
	}
}

int main(int argc, char *argv[])
{
	if(argc < 3)
		kill("USAGE: database <dbfile> <action> [action params]");

	char *filename = argv[1];
	char action = argv[2][0];
	struct Connection *conn = Database_open(filename,action);
	int id = 0;

	if(argc > 3) id = atoi(argv[3]);
	if(id >= MAX_ROWS) kill("Não existem muitos registros.");
	
	switch(action){
		case 'c':
			Database_create(conn);
			Database_write(conn);
			break;
		case 'g':
			if(argc != 7)
				kill("Precisa de uma id para obter dados.");
			Database_get(conn,id);
			break;
		case 's':
			if(!argc != 9)
				kill("Precisa de uma id, nome, logradouro, cep, bairro e data para setar.");
			Database_set(conn,id,argv[3],argv[4],argv[5],argv[6]);
			Database_write(conn);
			break;
		case 'd':
			if(argc != 4)
				kill("Precisa de uma id para deletar.");
			Database_delete(conn,id);
			Database_write(conn);
			break;
		case 'l':
			Database_list(conn);
			break;
		default:
			kill("Ação inválida: c=create , g=get, s=set, d=del, l=list");	
	}

	Database_close(conn);

	return 0;

}
