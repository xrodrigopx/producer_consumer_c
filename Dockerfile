FROM alpine:3.20

# Instalar las herramientas necesarias para el desarrollo en C
RUN apk add --no-cache gcc musl-dev make gdb git vim

# Establecer el directorio de trabajo
WORKDIR /app

# Copiar el código fuente al contenedor
COPY src/ .

# Comando por defecto para iniciar el contenedor
CMD ["sh"]


# Paso 1: Construir la imagen
# docker build -t c-dev .

# Paso 2: Ejecutar el contenedor
# docker run -it --rm c-dev

# Paso 3: Compilar el código fuente
# gcc obligatorio_sinc_procesos.c
#
# Paso 4: Ejecutar el programa
# ./a.out
