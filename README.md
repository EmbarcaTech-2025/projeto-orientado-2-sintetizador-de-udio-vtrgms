
# Projetos de Sistemas Embarcados - EmbarcaTech 2025

Autor: Vitor Gomes

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Campinas, ___ de 2025

---

## Objetivo

Este projeto faz parte do desenvolvimento da segunda fase do curso **Embarcatech**. Para isto desenvolvi um programa que faz a leitura de um microfone e a reproduz usando buzzer, feito com a placa de desenvolvimento [BitDogLab](https://github.com/BitDogLab).

## Funcionamento

Este programa faz a leitura ADC do microfone e converte em PWM para reproduzir o áudio usando um buzzer.

## Componentes usados

- Raspberry Pi Pico W
- Microfone de eletreto → Pino 34 (GP28)
- Buzzer → Pino 27 (GP21)
- LED RGB → Vermelho no pino 17 (GP13), verde no pino 15 (GP11) e azul no pino 16 (GP12)

## Como reutilizar

É necessário instalar a [extensão oficial da Raspberry Pi Pico](https://github.com/raspberrypi/pico-vscode) no seu VSCode e criar um novo projeto C/C++.
Clone os arquivos presentes neste diretório e execute com sua placa conectada no modo BOOTSEL para compilar e importar o projeto.

> _Atente-se para a versão do SDK, este programa foi feito usando a 2.1.1_

## Mídia

[Vídeo de demonstração](https://youtu.be/QOfsCxUIjKw?si=CUcuoWSL8X3s5wew)

## Resultados

O resultado foi aceitável, apesar de que mesmo configurando a resolução o áudio não saiu tão suave quanto esperado. Porém acredito que foi satisfatório dado a dificuldade da aplicação.


---

## 📜 Licença
GNU GPL-3.0.

