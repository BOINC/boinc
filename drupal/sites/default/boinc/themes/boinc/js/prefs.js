
  function customPrefsListener() {
    $('.advanced-settings input, .advanced-settings select, .advanced-settings textarea').change(function(){
      $('#edit-prefs-preset-custom').attr('checked', true);
    });
  };

  $(document).ready(function(){
    customPrefsListener();
  });
