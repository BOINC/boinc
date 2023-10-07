
  function setupLabel() {
      if ($('.label_check input').length) {
          $('.label_check').each(function(){
              $(this).removeClass('selected');
          });
          $('.label_check input:checked').each(function(){
              $(this).parent('label').addClass('selected');
          });
      };
      if ($('.form-radios.fancy .form-item label input').length) {
          $('.form-radios.fancy .form-item:first-child label').addClass('first');
          $('.form-radios.fancy .form-item label').each(function(){
              $(this).removeClass('selected');
          });
          $('.form-radios.fancy .form-item label input:checked').each(function(){
              $(this).parent('.form-radios.fancy .form-item label').addClass('selected');
          });
      };
  };
  function fancyRadiosInit(){
      $('.label_check, .form-radios.fancy .form-item label').click(function(){
          setupLabel();
      });
      setupLabel();
  };
  $(document).ready(function(){
      $('body').addClass('has-js');
      fancyRadiosInit();
  });
