import org.stockfish.StockfishJNI;

public class StockfishTest {

    @org.junit.Test
    public void testStaticEval(){
        StockfishJNI stockfishJNI = new StockfishJNI();
        System.out.println(stockfishJNI.staticEvalCp());
    }
}
