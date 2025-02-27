Esse código foi desenvolvido para o projeto final da residência de sistema embarcados EmbarcaTech - Cepedi. Trata-se da utilização da maioria dos aprendizados do curso para implementar uma solução inteligente para um problema a escolha.

O Umidificador Inteligente é um aparelho capaz de regular o próprio consumo de água segundo o nível de água disponível em seu relatório e a umidade do ambiente.
O objetivo é ter um umidificador capaz de agir com intensidade perante ambientes secos e eficientemente caso contrário, tendo principalmente cuidado com a quantidade de água que possui disponível.
Ele envia alertas, quando necessário, ao seu usuário, mantendo-o informado do que está acontecendo em tempo real.

Em suma, a meta é idealizar um umidificador que não dependa do usuário para umidificar o ambiente com eficiência, esperando que ele acerte a intensidade ideal.

//-----------------------------------------------------------------------------------------------------------------------------//

**Suas funcionalidades incluem:**
- O uso da matriz de LEDs WS2812, representando a quantidade de água no reservatório em nívels de 100%, 75%, 50%, 25%, 10% e 0%;
- O uso de LEDs RGB para simular o consumo de água da bomba do umidificador e sua intensidade;
- Implementação de rotina de interrupção quando o botão B é pressionado para abastecer o reservatório com 10% de sua carga máxima;
- Implementação de rotina de temporizador para checar o nível de água e umidade e definir a intensidade do umidificador com base nesses dados;
- Implementação de um ADC utilizando um Joystick, que irá alterar a umidade do ambiente segundo seu eixo Y;
- Utilização do display ssd1306 via i2c para exibir em tempo real a porcentagem de umidade do ambiente e o nível de água do reservatório;
- Utilização do monitor serial via UART para enviar mensagens de alerta ao usuário.

//-----------------------------------------------------------------------------------------------------------------------------//

**Como a intensidade é regulada?**

O Umidificador Inteligente prioriza estar ativo por mais tempo, então o nível de água é o fator principal para regular sua intensidade.
Desde que ele esteja abaixo de 50%, a intensidade da nebulização do umidificador mantem-se em 25% até que não haja mais água no reservatório, para quando vai para 0%.
O usuário é alertado de ambas situações para que possa prontamente tomar a devida solução: Reabastecer o reservatório.

Caso ele esteja acima desse nível, então a umidade é o fator regulador. Se ela estiver acima de 50%, a intensidade de nebulização também será 50%.
Caso esteja moderada, entre 30% e 50%, o ar está mais seco, então a intensidade aumenta para 75%.
E se estiver abaixo de 30%, é uma situação de risco, o consumo aumenta sua intensidade para 100%.

Os LEDs RGB e sua intensidade irão variar segundo a intensidade de consumo de água do umidificador.
Preferivelmente, o tempo de decremento do nível de água deveria ser mais rápido ou lento para uma demonstração mais realista, contudo para fins de simulação, essa alternativa foi utilizada.
Afinal, ficar esperando por mais tempo para verificar as variações no funcionamento do umidificador seria tedioso!

//-----------------------------------------------------------------------------------------------------------------------------//

**Como seria o projeto na prática?**

Utilizaria-se um microcontrolador capaz de se conectar com a internet via Wi-Fi e com módulo PWM para enviar pulsos pelos seus pinos.
Contaria com um protocolo de comunicação, a exemplo MQTT, para se comunicar com um aplicativo e mandar dados e avisos ao usuário.
Ele teria sensores de nível de água e umidade, e um pino acionado como PWM iria se conectar a bomba de água de um umidificador.
O umidificador, por sua vez, teria alimentação externa.

Claro, não haveria regulagem da umidade do ambiente por um joystick, a natureza ou aparelhos domésticos cuidam disso!

E o usuário ainda teria que abastecer o umidificador manualmente, a não ser que obtenha um que seja fixo num local.
Nesse caso, um sistema de abastecimento automático poderia ser feito utilizando esse projeto como base.

//-----------------------------------------------------------------------------------------------------------------------------//

**Vídeo Demonstrativo**

Para visualizar o experimento na BitDogLab, basta acessar o link a seguir:
[https://drive.google.com/file/d/1ddwNYEEDfIg010oAj2PnYLJRTr_oNFVf/view?usp=sharing]
