$('form.login').on('submit', function(){
    console.log("Trigger!");

    var obj = $(this);
    var url = obj.attr('action');
    var type = obj.attr('method');
    var data = {};

    obj.find('[name]').each(function(index, value){
        var obj = $(this);
        var name = obj.attr('name');
        var value = obj.val();

        data[name] = value;
    });

    console.log("|||"+JSON.stringify(data)+"|||");

    $.ajax({
        url: url,
        type: "GET",
        data: JSON.stringify(data),
        processData: false,
        success: function(response) {
            console.log("|||"+response+"|||");
        }
    })


    return false;
});