// Get the link to the previous page
var previousUrl = document.referrer;

// Check if document.referrer is available
if (previousUrl) {
  // Check if the previous page and the current one are in the same domain
  if (new URL(previousUrl).origin === window.location.origin) {
    fetch(previousUrl)
      .then((response) => response.text())
      .then(() => {
        // Get the target of the previous page
        var matches = previousUrl.match(/esp32[0-9a-z]*|other/);
        var previousTarget = matches && matches.length ? matches[0] : null;

        // Get the target of the current page
        var currentUrl = window.location.href;
        matches = currentUrl.match(/esp32[0-9a-z]*|other/);
        var currentTarget = matches && matches.length ? matches[0] : null;

        // If both targets are found, proceed with URL modification
        if (previousTarget && currentTarget) {
          // Get the base URL before the target
          var baseUrl = previousUrl.split(previousTarget)[0];

          // Construct the new URL to the target's homepage
          var newUrl = `${baseUrl}${currentTarget}/index.html`;

          console.log("Redirecting to:", newUrl);

          // Perform the redirect
          window.location.href = newUrl;
        } else {
          console.log("Targets not found, no redirection will occur.");
        }
      })
      .catch((error) => {
        console.error("Error fetching or parsing previous page:", error);
      });
  } else {
    console.log("Previous URL is from a different domain or unavailable.");
  }
} else {
  console.log("No referrer detected.");
}
