// From gocurrency

var currency = new Array("TRY","AFA","DZD","USD","AOA","ANG","ARS","AMD","AWG","AUD","AZM","XOF","XAF","BSD","BBD","BYR","BZD","BMD","BTN","BOB","BAM","BWP","BRL","GBP","BND","BGN","BIF","XPF","KHR","CAD","KYD","CLP","CNY","COP","KMF","CDF","CRC","HRK","CUP","CYP","CZK","DKK","DJF","DOP","XCD","EGP","SVC","ERN","EEK","ETB","EUR","FKP","FJD","GMD","GEL","GHC","GIP","GTQ","GNF","GYD","HTG","HNL","HKD","HUF","IRR","ISK","INR","IDR","IQD","ILS","JMD","JPY","JOD","KZT","KES","KGS","KWD","LAK","LVL","LBP","LSL","LRD","LYD","LTL","MOP","MKD","MGA","MWK","MYR","MVR","MTL","MRO","MUR","MXN","MDL","MNT","MAD","MZM","MMK","NAD","NPR","NZD","NIO","NGN","KPW","NOK","OMR","PKR","PAB","PGK","PYG","PEN","PHP","PLN","CVE","QAR","ROL","RUB","RWF","SHP","STD","SAR","CSD","SCR","SLL","SGD","SKK","SIT","SBD","SOS","WST","ZAR","KRW","LKR","SDD","SRD","SZL","SEK","CHF","SYP","TWD","TZS","THB","TOP","TTD","TND","TRL","TMM","UGX","UAH","UYU","AED","UZS","VUV","VEB","VND","YER","ZMK","ZWD");
var country = new Array("","AF","DZ","US","AO","AN","AR","AM","AW","AU","AZ","XO","XA","BS","BB","BY","BZ","BM","BT","BO","BA","BW","BR","GB","BN","BG","BI","XP","KH","CA","KY","CL","CN","CO","KM","CD","CR","HR","CU","CY","CZ","DK","DJ","DO","XC","EG","SV","ER","EE","ET","EU","FK","FJ","GM","GE","GH","GI","GT","GN","GY","HT","HN","HK","HU","IR","IS","IN","ID","IQ","IL","JM","JP","JO","KZ","KE","KG","KW","LA","LV","LB","LS","LR","LY","LT","MO","MK","MG","MW","MY","MV","MT","MR","MU","MX","MD","MN","MA","MZ","MM","NA","NP","NZ","NI","NG","KP","NO","OM","PK","PA","PG","PY","PE","PH","PL","CV","QA","RO","RU","RW","SH","ST","SA","CS","SC","SL","SG","SK","SI","SB","SO","WS","ZA","KR","LK","SD","SR","SZ","SE","CH","SY","TW","TZ","TH","TO","TT","TN","TR","TM","UG","UA","UY","AE","UZ","VU","VE","VN","YE","ZM","ZW");
var rate = new Array("","43","72.12","1","80.1823","1.78","3.0375","447","1.79","1.34084","4588","528.13","527.06","1","0.37697","1.99","2149.35","1.96","0.98","44.7","7.95","1.5811","5.40541","2.129","0.55944","1.588","1.5727","976","96.2","4005","1.1314","0.82","516.25","8.0095","2352","397.75","425","506.76","5.878","1","0.4633","22.842","5.9995","175.15","32.3","2.67","5.7498","1","13.5","12.6076","8.6896","0.80425","0.62688","1.75439","27.9","1.8098","9100","0.55975","7.572","4450","190","40.9","18.895","7.7531","212.16","9130","74.59","44.88","8805","1469.2","4.5245","62","114.81","0.7085","124.12","71.25","40.9126","0.29201","9950","0.5595","1501","5.95","54","1.3265","2.7767","8.0061","49.47","2175","137.85","3.6415","12.6","0.34594","263.49","30.62","11.1495","12.99","1192","8.8699","26900","6.42","6.0635","71.65","1.60051","17.11","127.5","143.05","6.3052","0.38498","59.95","1","3.08928","5730","3.315","51.83","3.1174","89.3","3.6398","29766","27.402","549.98","0.55975","6940","3.7504","69.73","5.1975","2350","1.5872","29.894","192.65","7.57576","1440","2.89855","6.075","945","102.64","223.5","2.71","6.039","7.4922","1.2674","51.91","31.943","1210","37.63","2.03252","6.269","1.3284","1.325","1345000","5200","1824","5.04","23.95","3.6727","1220.94","111.1","2144.6","15936","196.25","3080","99201.6");

var fromFlag = new Array(3,23,9,50,125,71); var nVal = 1;
var toFlag   = new Array(50,23,71,66,9,125);

function numberFormat() {
    var fltNum = document.calcForm.outV.value;
    var intNum = document.calcForm.outV.value;
    intNum = intNum.replace(',','');
    intNum = parseFloat(intNum);
    if(fltNum.indexOf('.') > 0 )
    {
        var dec = fltNum.substr(fltNum.indexOf('.')+1,2);
        dec = parseInt(dec);
        if(dec < 10)
            dec = dec * 10;
    } else {
        var dec = "00";
    }
    document.calcForm.outV.value = intNum;
}

function Cvalue()
{
  var fromR, toR, resultV;
  fromR = rate[parseInt(document.calcForm.from.value)];
  toR = rate[document.calcForm.to.selectedIndex];
  nVal = document.calcForm.inV.value;
  
  if ( IsNumeric(nVal) == false ) {
    alert("amount to multiply is not a number\n\nyou can only use\n\n1234567890 and . (dot)");
  }
  
  resultV = nVal * ( toR / fromR );

  // 6 relevant digits only, or integer 
  if ( (resultV == parseInt(resultV)) || (resultV > 99999) )
  {
    // mostly integer
    resultV = parseInt( resultV );
  }
  else
  {
    if (resultV > 1)
    {
    resultV = resultV.toString();
    resultV = resultV.substring(0,7);
    } else {
    resultV = resultV.toString();
    resultV = resultV.substring(0,8);
    }
  }

 
  document.calcForm.outV.value = "   " + comma(resultV) + " " + currency[document.calcForm.to.selectedIndex];
}

function comma(num)
{
 var n = Math.floor(num);
 var myNum = num + "";
 var myDec = ""
 
 if (myNum.indexOf('.',0) > -1){
  myDec = myNum.substring(myNum.indexOf('.',0),myNum.length);
 }
 var arr=new Array('0'), i=0; 
 while (n>0) 
   {arr[i]=''+n%1000; n=Math.floor(n/1000); i++;}
 arr=arr.reverse();
 for (var i in arr) if (i>0) //padding zeros
   while (arr[i].length<3) arr[i]='0'+arr[i];
 return arr.join() + myDec;
}

function IsNumeric(strString)
{
   var strValidChars = "0123456789.";
   var strChar;
   var blnResult = true;

   for (i = 0; i < strString.length && blnResult == true; i++)
   {
      strChar = strString.charAt(i);
      if (strValidChars.indexOf(strChar) == -1)
      {
         blnResult = false;
      }
   }
   return blnResult;
}
