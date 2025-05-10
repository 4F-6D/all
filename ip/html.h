#ifndef HTML_H
#define HTML_H

const char html_page[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Weight Scale</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      font-size: 70px; /* Increase the base font size */
    }
    h1 {
      font-size: 76px; /* Increase the size of the header text */
    }
    p {
      font-size: 64px; /* Increase the size of the paragraph text */
    }
  </style>
</head>
<body>
  <h1>Weight Scale</h1>
  <p id="weight">Loading...</p>
  <script>
    function fetchWeight() {
      fetch('/readweight')
        .then(response => response.text())
        .then(data => {
          document.getElementById('weight').innerText = "Weight: " + data + "gm";
        });
    }
    setInterval(fetchWeight, 1000); // Fetch weight every second
    window.onload = fetchWeight;
  </script>
</body>
</html>
)rawliteral";

#endif
