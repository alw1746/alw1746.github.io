//xchart test program with integers or dates on X-axis.
import org.knowm.xchart.*;
import org.knowm.xchart.style.*;
import org.knowm.xchart.style.markers.SeriesMarkers;
import org.knowm.xchart.style.lines.SeriesLines;
import org.knowm.xchart.style.Styler.ChartTheme;
import java.text.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Date;

public class XChart {
  
  public static void main(String[] args) {
    String series="ABC";

    try
    {

        //List<Date> x = new ArrayList<Date>();              //List of dates
        List<Integer> x = new ArrayList<Integer>();      //List of ints
        List<Double> y = new ArrayList<Double>();
    
        XYChart chart = new XYChartBuilder().width(1280).height(720).theme(ChartTheme.XChart)
        .title("XChart").xAxisTitle("Date").yAxisTitle("%Diff ").build();
        chart.getStyler().setPlotBackgroundColor(java.awt.Color.BLACK);
        chart.getStyler().setPlotMargin(0);
        chart.getStyler().setLegendPosition(Styler.LegendPosition.OutsideE);
        chart.getStyler().setXAxisLabelRotation(90);
        chart.getStyler().setYAxisGroupPosition(1,Styler.YAxisPosition.Right);

        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd");
/*
        x.add(sdf.parse("2020-10-19"));     //dates on X
        x.add(sdf.parse("2020-10-20"));
        x.add(sdf.parse("2020-10-21"));
        x.add(sdf.parse("2020-10-22"));
        x.add(sdf.parse("2020-10-23"));
        x.add(sdf.parse("2020-10-24"));
        x.add(sdf.parse("2020-10-25"));
        x.add(sdf.parse("2020-10-26"));
        x.add(sdf.parse("2020-10-27"));
        x.add(sdf.parse("2020-10-28"));
*/
        x.add(0);                           //ints on X
        x.add(1);
        x.add(2);
        x.add(3);
        x.add(4);
        x.add(5);
        x.add(6);
        x.add(7);
        x.add(8);
        x.add(9);

        y.add(2.1);
        y.add(4.5);
        y.add(3.2);
        y.add(5.6);
        y.add(2.5);
        y.add(3.8);
        y.add(5.1);
        y.add(7.4);
        y.add(4.8);
        y.add(2.7);
          
        XYSeries xyseries = chart.addSeries(series, x, y);
        xyseries.setMarker(SeriesMarkers.NONE);
        xyseries.setYAxisGroup(1);
        BitmapEncoder.saveBitmap(chart,"XChart.png", BitmapEncoder.BitmapFormat.PNG);

    } catch (Exception e) {
      e.printStackTrace();
    }
    
  }
  
}
