/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var i = 5;
$( document ).ready(redi);

function reloj(){
    
    var momentoActual = new Date() ;
    var segundo = momentoActual.getSeconds();
    
    var horaImprimible = " Esta pagina se redigira en " + i +"  segundos" ;
    i = i - 1;
    document.getElementById("123").innerHTML=horaImprimible;
    setTimeout("reloj()",1000);

}

function redi(){
    
    reloj();
   
    setInterval(function(){/* codigo a ejecutar*/
    
   
    window.location.replace("http://localhost:8084/ProyectoRedes2/");
    
    
    
    }, 5000);
    
}

