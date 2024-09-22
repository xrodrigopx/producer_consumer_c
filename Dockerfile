FROM alpine:3.20

# Instalar las herramientas necesarias para el desarrollo en C
RUN apk add --no-cache gcc musl-dev make gdb git vim

# Establecer el directorio de trabajo
WORKDIR /app

# Copiar el código fuente al contenedor
COPY src/ .

# Comando por defecto para iniciar el contenedor
CMD ["sh"]