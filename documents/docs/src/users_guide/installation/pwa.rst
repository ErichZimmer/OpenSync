PWA Installation
================

The OpenSync Synchronizer PWA is an installable browser application. To run it
from the repository, use Node.js as specified by ``software/pwa/package.json``
(currently version ``>=22.23.2``). From the repository root:

.. code-block:: console

   cd software/pwa
   npm ci
   npm run dev -- --app opensync-pwa --http --port 8000

Open the local URL printed by the server in a browser that supports Web Serial.
Use the browser's **Install app** command or install icon to add OpenSync as an
application. The installed app opens in its own window. To update the PWA in the
case of future enhancements and contributions, there are two ways to accomplish
this.

One way is to run the npm command again and open the url in the **same** browser
used for the installation. Then, open up the OpenSync installed app and wait for
a few minuted. An update button should appear in the top left corner which would
update the app with the latest version once clicked. After the update, you can stop
the local http server from the command line used to run the npm command (Ctrl + c).

Another way is to go into the installed app settings of the computer. The installed
OpenSync app will appear as an ordinary application and can be uninstalled as such.
Then, run the npm command, open the link in a browser that supports Web USB, and
re-install the OpenSync app with the updated code.

For an introduction to the controls, see :doc:`PWA Basic Usage
<../basic_usage/pwa>`.
