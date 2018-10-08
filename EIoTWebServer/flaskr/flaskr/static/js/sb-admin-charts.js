// Chart.js scripts
// -- Set new default font family and font color to mimic Bootstrap's default styling
Chart.defaults.global.defaultFontFamily = '-apple-system,system-ui,BlinkMacSystemFont,"Segoe UI",Roboto,"Helvetica Neue",Arial,sans-serif';
Chart.defaults.global.defaultFontColor = '#292b2c';
// -- Area Chart Example
idLineChart = document.getElementById("myAreaChart");
chartDataInit = {
    labels: [],
    datasets: [{
      label: "Valor",
      lineTension: 0.3,
      backgroundColor: "rgba(2,117,216,0.2)",
      borderColor: "rgba(2,117,216,1)",
      pointRadius: 5,
      pointBackgroundColor: "rgba(2,117,216,1)",
      pointBorderColor: "rgba(255,255,255,0.8)",
      pointHoverRadius: 5,
      pointHoverBackgroundColor: "rgba(2,117,216,1)",
      pointHitRadius: 20,
      pointBorderWidth: 2,
      data: [],
    }],
    options:opciones
  };
var chartLineArea = new Chart(idLineChart, {type: 'line', data: chartDataInit});

var selectedTable = "";
var selectedDevice = "";
var selectedVar = "";

$("#seccionGrafEiot").hide();
$("#seccionGrafRefrig").hide();
var datosGraf=[];
var etiqGraf=[];
var dictCompletoEiot=[];
var dictCompletoRefrig=[];
var etiqRegistros=[];
var datosRegistros=[];
var datosActualesGrafica=[];
var etiqActualesGrafica=[];

function cargarVariables(dictEiot,dictRefrig,listasRegistros)
{
  dictCompletoEiot=dictEiot;
  dictCompletoRefrig=dictRefrig;
  etiqRegistros=listasRegistros[0];
  datosRegistros=listasRegistros[1];
}

function modGraficas(idSh,etiq,datos)
{
  return function(){
    switch(idSh)
    {
      case "listaGraficasReg":
      $("#seccionGrafEiot").hide();
      $("#seccionGrafRefrig").hide();
      $("#tituloGrafica").text("Gráfica Registros");
      $("#botonDispEiot").text("Seleccionar dispositivo");
      $("#botonDispRefrig").text("Seleccionar refrigerador");
      $("#botonVarEiot").text("Variable EIoT");
      $("#botonVarRefrig").text("Variable Refrigerador");
      loadLineChart(etiqRegistros,datosRegistros);
      break;
      case "listaGraficasEiot":
      $("#seccionGrafEiot").show();
      $("#seccionGrafRefrig").hide();
      $("#tituloGrafica").text("Gráfica EIoT");
      $("#botonDispEiot").text("Seleccionar dispositivo");
      $("#botonDispRefrig").text("Seleccionar refrigerador");
      $("#botonVarEiot").text("Variable EIoT");
      $("#botonVarRefrig").text("Variable Refrigerador");
      break;
      case "listaGraficasRef":
      $("#seccionGrafEiot").hide();
      $("#seccionGrafRefrig").show();
      $("#tituloGrafica").text("Gráfica Refrigeradores");
      $("#botonDispEiot").text("Seleccionar dispositivo");
      $("#botonDispRefrig").text("Seleccionar refrigerador");
      $("#botonVarEiot").text("Variable EIoT");
      $("#botonVarRefrig").text("Variable Refrigerador");
      break;
    }
  };
}

$("#listaGraficasReg").click(function(){
  modGraficas("listaGraficasReg",0,0);
  selectedTable = "records";

});


$("#listaGraficasEiot").click(function(){
  modGraficas("listaGraficasEiot",0,0);
  selectedTable = "eiot";
});

$("#listaGraficasRef").click(function(){
  modGraficas("listaGraficasRef",0,0);
  selectedTable = "refrigeradores";
});

$("#cantidadEntradas5").click(function(){
  loadLineChart(etiqActualesGrafica,datosActualesGrafica,5);
});
$("#cantidadEntradas10").click(function(){
  loadLineChart(etiqActualesGrafica,datosActualesGrafica,10);
});
$("#cantidadEntradas20").click(function(){
  loadLineChart(etiqActualesGrafica,datosActualesGrafica,20);
});
$("#cantidadEntradas50").click(function(){
  loadLineChart(etiqActualesGrafica,datosActualesGrafica,50);
});
$("#cantidadEntradasDefecto").click(function(){
  loadLineChart(etiqActualesGrafica,datosActualesGrafica,200);
});


var opciones = {
    scales: {
      xAxes: [{
        time: {
          unit: 'date'
        },
        gridLines: {
          display: false
        },
        ticks: {
          maxTicksLimit: 7
        }
      }],
      yAxes: [{
        /*ticks: {
          min: 0,
          max: 40000,
          maxTicksLimit: 5
        },*/
        gridLines: {
          color: "rgba(0, 0, 0, .125)",
        }
      }],
    },
    legend: {
      display: false
    }
  };
var idLineChart;
var chartLineArea;
var areaChartData;

function loadLineChart(etiq,datos,length)
{
  console.log("Tabla seleccionada: " + selectedTable);
  console.log("Dispositivo seleccionado: " + selectedDevice);
  console.log("Variable seleccionada: " + selectedVar);
	datosActualesGrafica=datos;
    etiqActualesGrafica=etiq;
    
    if (length)
    {
      datos=datos.slice(Math.max(datos.length - length, 1));
      etiq=etiq.slice(Math.max(etiq.length - length, 1));
    }
	
	areaChartData = {
    labels: etiq,
    datasets: [{
      label: "Valor",
      lineTension: 0.3,
      backgroundColor: "rgba(2,117,216,0.2)",
      borderColor: "rgba(2,117,216,1)",
      pointRadius: 5,
      pointBackgroundColor: "rgba(2,117,216,1)",
      pointBorderColor: "rgba(255,255,255,0.8)",
      pointHoverRadius: 5,
      pointHoverBackgroundColor: "rgba(2,117,216,1)",
      pointHitRadius: 20,
      pointBorderWidth: 2,

      data: datos,
    }],
    options:opciones
  };
  
  idLineChart = document.getElementById("myAreaChart");
  //chartLineArea = new Chart(idLineChart, {type: 'line', data: areaChartData});
  chartLineArea.data=areaChartData;
  chartLineArea.update();
  
  return;
}

function loadLineChartListEiot(key,value){
  return function()
  {
    $("#botonVarEiot").text(value);
    tablaSlc=dictCompletoEiot[key+value];
    tablaContenido=tablaSlc[0];
    listaDatos=[];
    listaEtiq=[];
    for (var item in tablaContenido) {
      for (var i=0;i<tablaContenido[item].length;i++)
      {
        if (i == 0)
        {
          listaDatos.push(tablaContenido[item][i]);
        }else
        {
          listaEtiq.push(tablaContenido[item][i]);
        }
      }
    }
    loadLineChart(listaEtiq,listaDatos);
  };
}

function loadLineChartListRefrig(key,value){
  return function()
  {
    $("#botonVarRefrig").text(value);
    tablaSlc=dictCompletoRefrig[key+value];
    tablaContenido=tablaSlc[0];
    listaDatos=[];
    listaEtiq=[];
    for (var item in tablaContenido) {
      for (var i=0;i<tablaContenido[item].length;i++)
      {
        if (i == 0)
        {
          listaDatos.push(tablaContenido[item][i]);
        }else
        {
          listaEtiq.push(tablaContenido[item][i]);
        }
      }
    }

    loadLineChart(listaEtiq,listaDatos);
  };
}

function organizarDDmenus(liDispEiot,varDispEiot,liDispRefrig,varDispRefrig)
{
  $("#liDispEiot").empty();
  $("#liDispRefrig").empty();
  $("#liVarDispEiot").empty();
  $("#liVarDispRefrig").empty();


  for (var idx=0;idx<liDispEiot.length;idx++)
  {
    $("#liDispEiot").append('<li role="presentation" id="ListaGraficaEiot' + liDispEiot[idx] + '" ><a class="dropdown-item linkddmenu" href="#">' + liDispEiot[idx] +'</a></li>');
  }

  for (var item=0;item<liDispRefrig;item++)
  {
    $("#liDispEiot").append('<li role="presentation" id="ListaGraficaRefrig' + liDispRefrig[item] + '" ><a class="dropdown-item linkddmenu" href="#">' + liDispRefrig[item] +'</a></li>');
  }
  return;
}

function asignarVarsListas(liIDsEiot,dispEiot,varxDispEiot,liIDsRefrig,dispRefrig,varxDispRefrig)
{

  function clickListaEiot(key,values){
    return function(){

      $("#liVarDispEiot").empty();
      $("#botonDispEiot").text(key);
      selectedDevice = key;
      $("#botonVarEiot").text("Variable EIoT");
      for (var i=0;i<values.length;i++){
        $("#liVarDispEiot").append('<li role="presentation" id="listaGrafVarEiot' + values[i] + '"><a class="dropdown-item linkddmenu" href="#">'+ values[i] +'</a></li>');

        $("#listaGrafVarEiot" + values[i]).click(function(){

          loadLineChartListEiot(key,values[i]);
          selectedVar=values[i];
        });
      }
      
    };
  }
  function clickListaRefrig(key,values){
    return function(){
      $("#liVarDispRefrig").empty();
      $("#botonDispRefrig").text(key);
      $("#botonVarRefrig").text("Variable Refrigerador");

      for (var i=0;i<values.length;i++){

        $("#liVarDispRefrig").append('<li role="presentation" id="listaGrafVarRefrig' + values[i] + '"><a class="dropdown-item linkddmenu" href="#">'+ values[i] +'</a></li>');

        $("#listaGrafVarRefrig" + values[i]).click(loadLineChartListRefrig(key,values[i]));
      }
      
    };
  }


  for (var y=0;y<liIDsEiot.length;y++)
  {
    key=dispEiot[y];
    values=varxDispEiot[dispEiot[y]];
    $('#' + liIDsEiot[y]).bind('click', { key: "", values: "" }, function(event) {
      var data = event.data;

    });
    $('#' + liIDsEiot[y]).click(clickListaEiot(key,values));
  }

  for (var y=0;y<liIDsRefrig.length;y++)
  {
    key=dispRefrig[y];
    values=varxDispRefrig[dispRefrig[y]];
    $('#' + liIDsRefrig[y]).bind('click', { key: "", values: "" }, function(event) {
      var data = event.data;

    });
    $('#' + liIDsRefrig[y]).click(clickListaRefrig(key,values));
  }
  return;
}



// -- Bar Chart Example
var ctx = document.getElementById("myBarChart");
var myLineChart = new Chart(ctx, {
  type: 'bar',
  data: {
    labels: ["January", "February", "March", "April", "May", "June"],
    datasets: [{
      label: "Revenue",
      backgroundColor: "rgba(2,117,216,1)",
      borderColor: "rgba(2,117,216,1)",
      data: [4215, 5312, 6251, 7841, 9821, 14984],
    }],
  },
  options: {
    scales: {
      xAxes: [{
        time: {
          unit: 'month'
        },
        gridLines: {
          display: false
        },
        ticks: {
          maxTicksLimit: 6
        }
      }],
      yAxes: [{
        ticks: {
          min: 0,
          max: 15000,
          maxTicksLimit: 5
        },
        gridLines: {
          display: true
        }
      }],
    },
    legend: {
      display: false
    }
  }
});



// -- Pie Chart Example
var ctx = document.getElementById("myPieChart");
var myPieChart = new Chart(ctx, {
  type: 'pie',
  data: {
    labels: ["Blue", "Red", "Yellow", "Green"],
    datasets: [{
      data: [12.21, 15.58, 11.25, 8.32],
      backgroundColor: ['#007bff', '#dc3545', '#ffc107', '#28a745'],
    }],
  },
});
