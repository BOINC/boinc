
Description
===========
Token module provides a centralized API for text substitution. Unless
you're installing another Drupal module that requires it, this software
is probably unnecessary.

For more information on tokens see http://groups.drupal.org/tokens

Benefits
========
If you're a Drupal developer, check out API.txt for detailed instructions
on using the Token API. It allows every module to announce the placeholder
tokens they can handle, uses simple caching to prevent duplicated work in
a given page-view, and is pretty lightweight. It's nice. You'll like it.

tokenSTARTER
============
Want to add your own custom tokens to a site? Not sure how to write a 
module? Worry no more, it's now quite easy.

1. Copy and rename the tokenSTARTER.info and tokenSTARTER.module replacing
   every instance of STARTER with a descriptive, appropriate word.

2. Edit the .module file and change hook_token_list and hook_token_values
   to provide whatever additional tokens or logic your site needs.

3. Enable the module and enjoy!

You should also want to read the API.txt.