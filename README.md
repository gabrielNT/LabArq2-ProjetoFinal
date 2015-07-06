#Projeto Final - Arq 2
  Repositório para armazenar o trabalho final de Laboratório de Arquitetura e Organização de Computadores 2. O trabalho consistiu na paralelização do algoritmo de mandelbrot(para gerar um fractal num arquivo no formato ppm), utilizando principalmente instruções SIMD/*Intrinsics* no conjunto de instruções SSE e AVX, de forma a observar as implicações no desempenho.
  Também foi feita uma versão básica de paralelização utilizando OpenMP.

# Considerações Iniciais

  * As otimizações do código foram feitas com base na versão inicial do algoritmo, fornecida pelo professor e disponível no [link](https://github.com/menotti/arq2asm/blob/master/pbm/mandelbrot.c). Esse código também está no repositório, com o nome de *mandelbrot.c*.
  * Foi tentado não mudar a lógica original do algoritmo, com exceção nas partes que a paralelização do problema necessitavam disso explicitamente. Dessa forma, isso se reflete principalmente em dois pontos: não foram utilizados metódos para gerar o fractal diferente do proposto(uma implementação a partir de um *escape time*), mesmo que isso pudesse facilitar/melhorar a implementação, e a maneira de escrita da imagem na memória continuou como estava(escrevendo cada pixel logo após calculá-lo, o que em função dos acessos a disco constantes atrasam o tempo de execução consideravelmente).
  * No algoritmo original, foram utilizados variáveis de ponto flutuante de tipo *double*(64 bits). A fim de aproveitar melhor os registradores disponíveis e explorar os efeitos da paralelização, foi decidido trocá-las por variáveis de precisão simples(*float* - 32 bits). 
  * Foram utilizados 3 computadores para a execução dos testes. Foram considerados relevantes para discussão apenas os processadores e as quantidades de memória disponíveis sendo eles os seguintes:
    * **PC1:**[Intel Core i7-2600(8MB Cache, 3.8Ghz, 4 Cores)](http://ark.intel.com/products/52213/Intel-Core-i7-2600-Processor-8M-Cache-up-to-3_80-GHz), 8 GB de RAM  
    * **PC2:**[Intel Core i5-540M(3MB Cache, 2.53Ghz, 2 Cores)](http://ark.intel.com/products/43544/Intel-Core-i5-540M-Processor-3M-Cache-2_53-GHz), 4 GB de RAM(Não possui conjunto de instruções AVX)
    * **PC3:**[Intel Core i3-2310M(3MB Cache, 2.10Ghz, 2 Cores)](http://ark.intel.com/products/52220/Intel-Core-i3-2310M-Processor-3M-Cache-2_10-GHz), 4 GB de RAM
  * Para os testes de desempenho, os programas foram executados 5 vezes, utilizando uma média aritmética, para minimizar as eventuais flutuações nos valores.
  * Foram utilizadas instruções SIMD de carregamento não alinhadas. A alternativa parecia complicada demais para o escopo da proposta, e os efeitos disso(mais detalhados a seguir) não foram descobertos até muito tarde no desenvolvimento do projeto.

# Abordagem na Paralelização

  Durante o processo de paralelização, surgiram algumas alternativas em relação ao que paralelizar em cada programa. Basicamente, a primeira questão é determinar quais *loops* são paralelizáveis. O programa possui três aninhados: o primeiro(que varia iY) e o segundo(que varia iX) são, enquanto o segundo não é por possuir dependência em relação às iterações anteriores. A relação de qual deles realmente paralelizar(o primeiro, o segundo ou ambos) será melhor explorada nas seções seguintes.
  Para resolver as questões que serão levantadas a seguir, foram feitas alguns programas paralelizando o loop de iX com o conjunto SSE, de forma a observar o desempenho de cada um.
  É fácil perceber que o uso de instruções SIMD por *inline assembly* traz algumas consequências para a implementação, relacionadas ao tratamento dos dados na hora de alocar posições na memória e armazená-los novamente. Uma das instruções paralelizadas foi a seguinte:
  
  `Cx = CxMin + iX*PixelWidth;`
  
  Como se pode observar no código original, essa instrução não pode mais ser executada dessa forma, já que o *for* que anteriormente variava iX agora deverá executar 4 iterações por vez. Porntanto, a solução encontrada foi utilizar o seguinte vetor(com a dada lógica de preenchimento), de forma a ser usada nas operações SIMD:
  
  ``` C
  // Gera os indices e coloca em simdIx
			float simdIx[4];
			for (int i = 0; i < 4; i++)
				simdIx[i] = iX * 4.0 + i;
  ```
  
  Essa implentação está no arquivo *mandelbrotSSE_iX_2.c*. No entanto, foi discutido a possibilidade de implementar isso utilizando *inline assembly*(e se isso melhoraria a performance ou não, tendo em mente nosso conhecimento limitado das instruções da arquitetura). O trecho de código gerado foi o seguinte:
  
```C
  float simdIx[4];

			_asm{
					finit
					mov ecx, 0

					// Gera o vetor simdIx
				L1:
					fild iX
					fld valor4
					fmul
					mov counter, ecx
					fild counter
					fadd
					fstp simdIx[ecx]

					add ecx, 4
					cmp ecx, 16
					je FIM
					jmp L1

				FIM :

				// xmm5 = CxMin + iX*PixelWidth
				movups xmm5, simdIx
					mulps xmm5, PixelWidth128
					addps xmm5, CxMin128
					movups _Cx, xmm5
			}
```

Essa implentação se encontra no arquivo *mandelbrotSSE_iX_1.c*.

Outro problema encontrado está no seguinte loop:

``` C
for (int i = 0; i < 4; i++){
				color[0] = (char)((vetorCoresAux[i] % 8) * 63);  /* Red */
				color[1] = (char)((vetorCoresAux[i] % 4) * 127);  /* Green */
				color[2] = (char)((vetorCoresAux[i] % 2) * 255);  /* Blue */
				...
}
```

A operação módulo não está presente nas instruções SIMD(nem divisões inteiras). Implementar isso seria demasiadamente complicado, mas caso seja desejado o restante dos cálculos é possível de se realizar. Para fazê-lo, no entanto, é necessário guardar os valores novamente nos registradores SSE e, para calcular o resto da divisão, retornar novamente para C. Todo esse processo poderia causar um *overhead*, e dessa forma foi necessário testar como seria o desempenho sem fazer essa parte paralelamente, e para esse caso foi utilizado o arquivo *mandelbrotSSE_iX_3*, enquanto a versão paralela está em *mandelbrotSSE_iX_1.c*.

Os seguintes testes de execução foram feitos no PC1:

![Versões SSE](https://github.com/gabrielNT/LabArq2-ProjetoFinal/blob/master/Gr%C3%A1ficos/grafico2.png)

Como podemos ver, as versões 2 e 3 se demonstraram mais rápidas(apesar de não muito) do que a versão 1. Com esses resultados, as respectivas implementações foram aplicadas em *mandelbrotSSE_iX.c*, gerando uma execução ainda melhor. Os próximos programas também seguiram esse padrão nas execuções.

# Paralelização dos *loops* e Conjuntos de Instruções

A seguir, foi feita uma análise de como a paralelização dos diferentes *loops* afetavam o desempenho e também da implementação nos diferentes conjuntos de instruções. Vamos observar inicialmente os desempenhos com os programas na verão SSE:

![Versões SSE](https://github.com/gabrielNT/LabArq2-ProjetoFinal/blob/master/Gr%C3%A1ficos/grafico1.png)

![SSE - Barra](https://github.com/gabrielNT/LabArq2-ProjetoFinal/blob/master/Gr%C3%A1ficos/grafico1_barra.jpg)

Podemos observar que o desempenho foi similar em todas as versões, porém a original ainda foi a mais rápida(com uma variação entre paralelizar iX e iY). Levando em consideração flutuações na performance das máquinas na execução dos programas, é possível tirar poucas conclusões em relação a cada forma de paralelização. No geral, paralelizar ambos os *loops* se mostrou um pouco pior, o que deve acontecer devido ao *overhead* de acesso a memória aumentar, o que também é um dos motivos para a versão original ainda ser mais veloz. 

Outro ponto interessante é notar o quanto a diferença entre as especificações dos processadores afeta a performance, especialmente o *clock* e o tamanho da *cache* nesse caso.

Analisando a seguir o desempenho das versões em AVX(lembrando que PC2 não possui suporte à AVX):

![Versões AVX](https://github.com/gabrielNT/LabArq2-ProjetoFinal/blob/master/Gr%C3%A1ficos/grafico3.png)

Podemos ver que os desempenhos ainda são quase constantes, porém o desempenho em AVX no PC3 é muito inferior em dois dos casos, e as performances de AVX restantes são ligeiramente inferiores ao original e às versões SSE. Observando apenas os códigos apresentados, é difícil entender o motivo, já que as operações realizadas são analógas e o desempenho no PC1 conseguiu se manter. Um possível motivo encontrado está relacionado ao alinhamento dos dados na arquitetura *Sandy Bridge*, onde instruções dois dados não alinhados de 128 bits são significamente mais rápidos do que uma instrução de 256 bits[(como explicado no quarto post desse link)](https://software.intel.com/en-us/forums/topic/282731).

Explicaremos melhor isso no próximo tópico. Uma última consideração interessante aqui é que, mesmo com PC1 ainda possuindo um processador que é parte da *Sandy Bridge*, as especificações mais elevadas desse processador parecem minimizar esses efeitos. 

# Alinhamento de Dados

No [manual da Intel de otimização, seção 4.4,](http://www.intel.com.br/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf) podemos ver que o alinhamento dos dados na memória aumenta consideravelmente as performances das instruções SIMD. Para explicar o conceito brevemente, podemos dizer que o computador acessa a memória em blocos definidos, e para otimizar os processos é necessário fazer com que os dados do programa sejam carregados em múltiplos do tamanho desse bloco(mesmo que seja necessário desperdiçar algumas das posições de memória com dados inútei, ou de *padding*). No caso dos conjuntos SSE e AVX, o alinhamento necessário é de 16 bits. Um breve metódo de alinhar os vários tipos de dados pode ser visto [aqui](https://software.intel.com/en-us/articles/align-and-organize-data-for-better-performance).

Na nossa implementação dos programas, esse cuidado de alinhar os dados não foi tomado(como já observado nas Considerações Iniciais), e portanto foi necessário usar instruções de carregamento não alinhadas. Como pode ser visto no manual na seção já indicada, a performance dessas não é a ótima, o que pode explicar em parte os resultados encontrados(principalmente o fato de nenhuma das versões otimizadas ter apresentado desempenho superior ao original: o *overhead* gerado nas implementações parece não ter sido suficiente para justificar a paralelização).

# Versão OpenMP

A versão implementada em OpenMP(*mandelbrotOpenMP.c*) teve tempo de execução médio de 153,21 segundos no PC1. É possível perceber facilmente que este é muito maior que as versões SIMD e a original. Isso parece ter acontecido principalmente devido ao loop não paralelizável, já discutido anteriormente, para o qual foi necessário utilizar o parâmetro *#pragma ordered* a fim de executar aquele loop sequencialmente. Isso acabou causando um *overhead* muito grande a ponto da paralelização do *for* não valer mais a pena(já que, como se pode ver, essa é uma das partes que requerem mais recursos no programa), e não permitindo a divisão própria de trabalho dentro dos loops.

Foi experimentado com outros parâmetros de OpenMP, porém os efeitos desses não foram bons para o desempenho ou não funcionavam para o algoritmo. Chegamos a conclusão que, para melhorar o desempenho com essa técnica, seria melhor alterar antes a estrutura do programa a fim de poder aplicar mais facilmente(e eficientemente) os pragmas. Isso fugia do escopo proposto no projeto(como visto em uma das considerações iniciais), portanto decidimos não explorar tanto esse caminho.

# Conclusões

Apesar de não ter sido possível desenvolver uma versão claramente mais eficiente do algoritmo, algumas coisas interessantes(e importantes) puderam ser observadas no decorrer do trabalho. 

Um deles é o fato de que, a fim de paralelizar os programas, é essencial que se projete-o de forma a suportar e potencializar os efeitos das instruções paralelas(evidenciado principalmente pela versão OpenMP). Dessa maneira, notamos que seria interessante pensar no algoritmo desde o início de seu desenvolvimento com o objetivo de paralelizá-lo, com a meta de evitar construções que atrasam e diminuem os efeitos da execução concorrente.

Um outro seria notar como processadores mais poderosos influenciam no desempenho, destacando o efeito de suas especificações e arquiteturas com a execução de um programa simples(embora longo e intensivo).

Também é importante perceber como o conhecimento da arquitetura é importante no desenvolvimento de códigos mais eficientes(tanto em tempo quanto espaço), seja no domínio das instruções e técnicas relevantes para essa bem como no próprio funcionamento do sistema(como mostrado com o impacto gerado pelo alinhamento de dados).
