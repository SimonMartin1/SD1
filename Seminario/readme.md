Como correr Latex en Vs Code

<!-- 1. Se instalan el Motor de Latex (Este es para Windows) -->

winget install MiKTeX.MiKTeX

 <!-- 2. Reinician Vs Code y se instalan la extension de Vs Code LaTeX Workshop -->

 "MikTeX could not run "latexmk" because it need Pearl Instalattion"

<!-- Si les aparece un error como ese, siguen estos pasos -->

1. En tu archivo de LaTeX abierto en VS Code, presiona Ctrl + Shift + P para abrir la paleta de comandos.
2. Escribe LaTeX Workshop: Build con receta (o Build with recipe) y selecciónalo.
3. En la lista desplegable que te aparece arriba, elige la opción que dice:
pdflatex -> bibtex -> pdflatex x2

<!-- Cómo dejarlo automático para siempre -->

Para no tener que seleccionar esa receta manualmente cada vez que guardas, indícale a VS Code que esa sea tu receta predeterminada:
1. Presiona Ctrl + , (Control y coma) para abrir la Configuración de VS Code.
2. En la barra de búsqueda de arriba, escribe: latex-workshop.latex.recipe.default
3. Cambia el valor de first a lastUsed


<!-- Al buildear les genera el PDF que se actualiza cada vez que modifican -->
