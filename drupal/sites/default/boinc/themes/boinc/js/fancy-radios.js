
  function setupLabel() {
      if ($('.label_check input').length) {
          $('.label_check').each(function(){ 
              $(this).removeClass('selected');
          });
          $('.label_check input:checked').each(function(){ 
              $(this).parent('label').addClass('selected');
          });                
      };
      if ($('.form-radios .form-item label input').length) {
          $('.form-radios .form-item:first-child label').addClass('first');
          $('.form-radios .form-item label').each(function(){
              $(this).removeClass('selected');
          });
          $('.form-radios .form-item label input:checked').each(function(){ 
              $(this).parent('.form-radios .form-item label').addClass('selected');
          });
      };
  };
  $(document).ready(function(){
      $('body').addClass('has-js');
      $('.label_check, .form-radios .form-item label').click(function(){
          setupLabel();
      });
      setupLabel(); 
  });
